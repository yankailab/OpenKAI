#include "_YOLO26depthEstONNX.h"
#include "../UI/_Console.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace kai
{
    namespace
    {
        int roundImageSize(double value)
        {
            // Match Python round() for positive dimensions, including .5 ties.
            const int base = static_cast<int>(value);
            const double fraction = value - base;
            return base + (fraction > 0.5 || (fraction == 0.5 && base % 2 != 0));
        }

        bool depthShape(const vector<int64_t> &shape, bool dynamic)
        {
            if (shape.size() < 2 || shape.size() > 4)
                return false;
            for (size_t i = 0; i + 2 < shape.size(); ++i)
                if (shape[i] != 1 && !(dynamic && shape[i] < 0))
                    return false;
            for (size_t i = shape.size() - 2; i < shape.size(); ++i)
                if ((shape[i] <= 0 && !(dynamic && shape[i] < 0)) ||
                    shape[i] > std::numeric_limits<int>::max())
                    return false;
            return true;
        }

        // Ultralytics first restores reduced-resolution exports with
        // interpolate(align_corners=True), before removing letterbox padding.
        void restoreModelSize(const Mat &src, Mat &dst, const cv::Size &size)
        {
            if (src.size() == size)
            {
                dst = src;
                return;
            }
            dst.create(size, CV_32FC1);
            const double sx = size.width > 1 ? double(src.cols - 1) / (size.width - 1) : 0;
            const double sy = size.height > 1 ? double(src.rows - 1) / (size.height - 1) : 0;
            for (int y = 0; y < size.height; ++y)
            {
                const double fy = y * sy;
                const int y0 = static_cast<int>(fy), y1 = std::min(y0 + 1, src.rows - 1);
                const float wy = static_cast<float>(fy - y0);
                const float *a = src.ptr<float>(y0), *b = src.ptr<float>(y1);
                float *out = dst.ptr<float>(y);
                for (int x = 0; x < size.width; ++x)
                {
                    const double fx = x * sx;
                    const int x0 = static_cast<int>(fx), x1 = std::min(x0 + 1, src.cols - 1);
                    const float wx = static_cast<float>(fx - x0);
                    out[x] = (a[x0] * (1 - wx) + a[x1] * wx) * (1 - wy) +
                             (b[x0] * (1 - wx) + b[x1] * wx) * wy;
                }
            }
        }
    }

    _YOLO26depthEstONNX::_YOLO26depthEstONNX()
        : m_env(ORT_LOGGING_LEVEL_WARNING, "OpenKAI_YOLO26depthEstONNX"),
          m_memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
    {
    }

    _YOLO26depthEstONNX::~_YOLO26depthEstONNX()
    {
        // Join both workers while their frame storage and model still exist.
        stop();
        DEL(m_pTpp);
        DEL(m_pT);
    }

    bool _YOLO26depthEstONNX::loadConfig(void)
    {
        // No worker may read calibration or the model while either is replaced.
        stop();
        DEL(m_pTpp);
        IF_F(!_DetectorBase::loadConfig());
        m_pTpp = createThread(jK(*m_pJ, "threadPP"), "threadPP");
        NULL_F(m_pTpp);
        const json &j = *m_pJ;
        jKv<int>(j, "vModelInputSize", m_vModelInputSize);
        jKv(j, "bSwapRB", m_bSwapRB);
        jKv(j, "scale", m_scale);
        jKv(j, "nThread", m_nThread);
        jKv<float>(j, "vRangeD", m_vRangeD);
        jKv(j, "dScale", m_dScale);
        jKv(j, "dOfs", m_dOfs);
        jKv(j, "bPCL", m_bPCL);
        jKv(j, "bPCLrgb", m_bPCLrgb);
        jKv<float>(j, "vFocal", m_vFocal);
        jKv<float>(j, "vPrincipal", m_vPrincipal);
        jKv<int>(j, "vSizeCalib", m_vSizeCalib);
        jKv(j, "nPCLstep", m_nPCLstep);

        IF_Le_F(m_nThread < 0 || m_vModelInputSize.minCoeff() <= 0 ||
                    !std::isfinite(m_scale) || m_scale <= 0,
                "Invalid ONNX input size, scale or nThread (0 = automatic, positive = worker count)");

        IF_Le_F(!m_vRangeD.allFinite() || m_vRangeD.x() < 0 ||
                    m_vRangeD.y() <= m_vRangeD.x() || !std::isfinite(m_dScale) ||
                    m_dScale <= 0 || !std::isfinite(m_dOfs),
                "Invalid depth range, dScale or dOfs");

        IF_Le_F(m_nPCLstep < 1, "nPCLstep must be positive");

        IF_Le_F((m_bPCL || m_bPCLrgb) &&
                    (!m_vFocal.allFinite() || m_vFocal.minCoeff() <= 0 ||
                     !m_vPrincipal.allFinite() || m_vSizeCalib.minCoeff() <= 0),
                "Point clouds require calibrated vFocal, vPrincipal and vSizeCalib");

        m_tLastInput = 0;
        return loadModel();
    }

    bool _YOLO26depthEstONNX::saveConfig(bool bExport)
    {
        IF_F(!_DetectorBase::saveConfig(false));
        json &j = *m_pJ;
        j["vModelInputSize"] = {m_vModelInputSize.x(), m_vModelInputSize.y()};
        j["bSwapRB"] = m_bSwapRB;
        j["scale"] = m_scale;
        j["nThread"] = m_nThread;
        j["vRangeD"] = {m_vRangeD.x(), m_vRangeD.y()};
        j["dScale"] = m_dScale;
        j["dOfs"] = m_dOfs;
        j["bPCL"] = m_bPCL;
        j["bPCLrgb"] = m_bPCLrgb;
        j["vFocal"] = {m_vFocal.x(), m_vFocal.y()};
        j["vPrincipal"] = {m_vPrincipal.x(), m_vPrincipal.y()};
        j["vSizeCalib"] = {m_vSizeCalib.x(), m_vSizeCalib.y()};
        j["nPCLstep"] = m_nPCLstep;
        IF_F(m_pTpp && !m_pTpp->saveConfig(false));

        return !bExport || m_pJcfg->saveToFile();
    }

    bool _YOLO26depthEstONNX::link(InstanceMgr *pM)
    {
        // Depth detectors share model/RGB structures but do not produce boxes.
        IF_F(!_ModuleBase::link(pM));
        const json &j = *m_pJ;

        string n;
        jKv(j, "RGBframeIn", n);
        m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pRGBin, "RGBframeIn not found: " + n);

        n.clear();
        jKv(j, "RGBDframeOut", n);
        m_pDout = dynamic_cast<RGBDframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pDout, "RGBDframeOut not found: " + n);

        n.clear();
        jKv(j, "DframeOut", n);
        m_pDepthOut = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!n.empty() && !m_pDepthOut, "DframeOut not found: " + n);
        IF_Le_F(m_pDepthOut == m_pRGBin, "DframeOut must differ from RGBframeIn");

        n.clear();
        jKv(j, "PCLframeOut", n);
        m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F((!n.empty() || m_bPCL || m_bPCLrgb) && !m_pPCLout,
                "PCLframeOut not found: " + n);

        return true;
    }

    bool _YOLO26depthEstONNX::loadModel(void)
    {
        m_pSession.reset();

        try
        {
            m_sessionOptions.SetIntraOpNumThreads(m_nThread);

            // ALL also enables CPU layout optimizations (including NCHWc).
            m_sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

            auto session = std::make_unique<Ort::Session>(m_env, m_fModel.c_str(), m_sessionOptions);
            IF_Le_F(session->GetInputCount() != 1 || session->GetOutputCount() != 1,
                    "Depth ONNX must have exactly one image input and one depth output");

            const auto inType = session->GetInputTypeInfo(0);
            const auto outType = session->GetOutputTypeInfo(0);
            IF_Le_F(inType.GetONNXType() != ONNX_TYPE_TENSOR || outType.GetONNXType() != ONNX_TYPE_TENSOR,
                    "Depth ONNX input/output must be tensors");

            const auto inInfo = inType.GetTensorTypeAndShapeInfo();
            const auto outInfo = outType.GetTensorTypeAndShapeInfo();
            const auto inShape = inInfo.GetShape();

            IF_Le_F(inInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT ||
                        inShape.size() != 4 || (inShape[0] != 1 && inShape[0] >= 0) || inShape[1] != 3,
                    "Depth ONNX requires float32 NCHW input with batch 1 and 3 channels");

            IF_Le_F(outInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT ||
                        !depthShape(outInfo.GetShape(), true),
                    "Depth ONNX requires float32 [1,1,H,W], [1,H,W] or [H,W] output");

            for (int i = 2; i < 4; ++i)
            {
                IF_Le_F(inShape[i] == 0 || inShape[i] > std::numeric_limits<int>::max(),
                        "Invalid ONNX spatial input dimensions");
                if (inShape[i] > 0)
                    m_vModelInputSize[3 - i] = static_cast<int>(inShape[i]);
            }
            IF_Le_F(m_vModelInputSize.minCoeff() <= 0, "Dynamic ONNX requires vModelInputSize");

            Ort::AllocatorWithDefaultOptions allocator;
            const auto task = session->GetModelMetadata().LookupCustomMetadataMapAllocated("task", allocator);
            IF_Le_F(task && string(task.get()) != "depth", "ONNX model task must be depth");

            m_inputName = session->GetInputNameAllocated(0, allocator).get();
            m_outputName = session->GetOutputNameAllocated(0, allocator).get();
            m_pSession = std::move(session);
        }
        catch (const Ort::Exception &e)
        {
            LOG_E("ONNX Runtime depth load failed: " + string(e.what()));
            return false;
        }
        return true;
    }

    bool _YOLO26depthEstONNX::start(void)
    {
        IF_F(!check());
        IF_F(m_pT->bRun() || (m_pTpp && m_pTpp->bRun()));
        if (m_bPCL || m_bPCLrgb)
        {
            NULL_F(m_pTpp);
            if (!m_pTpp->startThread(getTPP, this))
            {
                stop();
                return false;
            }
        }
        if (!m_pT->startThread(getUpdate, this))
        {
            stop();
            return false;
        }
        return true;
    }

    void _YOLO26depthEstONNX::stop(void)
    {
        // Do not use check(): partial initialization must also be stoppable.
        if (m_pT)
            m_pT->stop();
        {
            std::lock_guard<std::mutex> lock(m_mtxPCL);
            if (m_pTpp)
                m_pTpp->stop();
        }
        m_cvPCL.notify_all();
        if (m_pT)
            m_pT->join();
        if (m_pTpp)
            m_pTpp->join();

        std::lock_guard<std::mutex> lock(m_mtxPCL);
        m_pendingRGB.release();
        m_pendingDepth.release();
        m_pendingStamp = 0;
    }

    _Thread *_YOLO26depthEstONNX::getThread(const string &name)
    {
        if (name == "threadPP")
            return m_pTpp;
        return _DetectorBase::getThread(name);
    }

    bool _YOLO26depthEstONNX::check(void)
    {
        return m_pSession && m_pRGBin && m_pDout &&
               (!(m_bPCL || m_bPCLrgb) || m_pPCLout) && _ModuleBase::check();
    }

    void _YOLO26depthEstONNX::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            detect();
        }
    }

    void _YOLO26depthEstONNX::detect(void)
    {
        IF_(!check());

        Mat input;
        const uint64_t stamp = m_pRGBin->get(input);
        IF_(!stamp || stamp == m_tLastInput || input.empty());

        m_tLastInput = stamp;
        Mat depth;
        IF_(!estimateDepth(input, depth));

        // Publish depth immediately; cloud generation overlaps the next inference.
        m_pDout->set(input, depth, stamp);
        if (m_pDepthOut)
            m_pDepthOut->set(depth, stamp);
        if (m_pPCLout && (m_bPCL || m_bPCLrgb))
            queuePointCloud(input, depth, stamp);
    }

    bool _YOLO26depthEstONNX::estimateDepth(const Mat &input, Mat &depth)
    {
        depth.release();
        IF_Le_F(!m_pSession || input.empty() || input.type() != CV_8UC3,
                "Depth inference requires a loaded model and CV_8UC3 BGR input");
        try
        {
            const int w = m_vModelInputSize.x(), h = m_vModelInputSize.y();
            const double gain = std::min(double(w) / input.cols, double(h) / input.rows);
            const int rw = std::max(1, std::min(w, roundImageSize(input.cols * gain)));
            const int rh = std::max(1, std::min(h, roundImageSize(input.rows * gain)));
            const int left = (w - rw) / 2, top = (h - rh) / 2;
            const cv::Rect content(left, top, rw, rh);
            Mat resized, padded;

            cv::resize(input, resized, cv::Size(rw, rh), 0, 0, cv::INTER_LINEAR);
            cv::copyMakeBorder(resized, padded, top, h - rh - top, left, w - rw - left,
                               cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));

            Mat floats;
            padded.convertTo(floats, CV_32FC3, m_scale);
            if (m_bSwapRB)
                cv::cvtColor(floats, floats, cv::COLOR_BGR2RGB);

            vector<Mat> channels;
            cv::split(floats, channels);
            const size_t plane = static_cast<size_t>(w) * h;
            vector<float> tensor(3 * plane);
            for (int c = 0; c < 3; ++c)
                std::memcpy(tensor.data() + c * plane, channels[c].ptr<float>(), plane * sizeof(float));

            const int64_t shape[] = {1, 3, h, w};
            auto image = Ort::Value::CreateTensor<float>(m_memoryInfo, tensor.data(), tensor.size(), shape, 4);
            const char *inputs[] = {m_inputName.c_str()}, *outputs[] = {m_outputName.c_str()};

            auto result = m_pSession->Run(Ort::RunOptions{nullptr}, inputs, &image, 1, outputs, 1);
            IF_Le_F(result.empty() || !result[0].IsTensor(), "Depth ONNX did not return a tensor");

            const auto info = result[0].GetTensorTypeAndShapeInfo();
            const auto dims = info.GetShape();
            IF_Le_F(info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || !depthShape(dims, false),
                    "Unexpected depth ONNX output shape/type");

            const int dh = static_cast<int>(dims[dims.size() - 2]);
            const int dw = static_cast<int>(dims.back());
            Mat raw(dh, dw, CV_32FC1, result[0].GetTensorMutableData<float>()), modelDepth;
            restoreModelSize(raw, modelDepth, cv::Size(w, h));
            cv::resize(modelDepth(content), depth, input.size(), 0, 0, cv::INTER_LINEAR);

            // The exported head already applies exp and its learned calibration.
            // Keep invalid values invalid even when a positive offset is configured.
            const float minDepth = m_vRangeD.x(), maxDepth = m_vRangeD.y();
            for (int y = 0; y < depth.rows; ++y)
            {
                float *row = depth.ptr<float>(y);
                for (int x = 0; x < depth.cols; ++x)
                {
                    const float rawDepth = row[x];
                    const float z = rawDepth * m_dScale + m_dOfs;
                    row[x] = std::isfinite(rawDepth) && rawDepth > 0 && std::isfinite(z) &&
                                     z > 0 && z >= minDepth && z <= maxDepth
                                 ? z
                                 : 0;
                }
            }
        }
        catch (const Ort::Exception &e)
        {
            LOG_E("ONNX Runtime depth inference failed: " + string(e.what()));
            depth.release();
            return false;
        }
        catch (const cv::Exception &e)
        {
            LOG_E("Depth image processing failed: " + string(e.what()));
            depth.release();
            return false;
        }
        return true;
    }

    void _YOLO26depthEstONNX::queuePointCloud(const Mat &rgb, const Mat &depth, uint64_t stamp)
    {
        {
            std::lock_guard<std::mutex> lock(m_mtxPCL);
            // detect() owns fresh Mats each time. Retaining their references
            // keeps this matched snapshot immutable without copying its pixels.
            m_pendingRGB = rgb;
            m_pendingDepth = depth;
            m_pendingStamp = stamp;
        }
        m_cvPCL.notify_one();
    }

    void _YOLO26depthEstONNX::updateTPP(void)
    {
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(m_mtxPCL);
                // The predicate retains work even if notification precedes wait.
                m_cvPCL.wait(lock, [this]
                             { return m_pendingStamp != 0 || !m_pTpp->bRun(); });
                if (!m_pTpp->bRun())
                    break;
            }
            // Track worker FPS without adding a delay to frame-ready wakeups.
            m_pTpp->skipSleep();
            m_pTpp->autoFPS();
            updatePCL();
        }
    }

    void _YOLO26depthEstONNX::updatePCL(void)
    {
        Mat rgb, depth;
        uint64_t stamp;
        {
            std::lock_guard<std::mutex> lock(m_mtxPCL);
            IF_(!m_pendingStamp);
            rgb = std::move(m_pendingRGB);
            depth = std::move(m_pendingDepth);
            stamp = m_pendingStamp;
            m_pendingStamp = 0;
        }

        // Inference only holds the handoff mutex briefly, never during PCL work.
        vector<GEOMETRY_POINT> cloud;
        makePointCloud(rgb, depth, stamp, cloud);
        if (m_pPCLout)
            m_pPCLout->set(cloud, stamp);
    }

    void _YOLO26depthEstONNX::makePointCloud(const Mat &rgb, const Mat &depth, uint64_t stamp,
                                             vector<GEOMETRY_POINT> &cloud) const
    {
        cloud.clear();
        const float sx = float(depth.cols) / m_vSizeCalib.x();
        const float sy = float(depth.rows) / m_vSizeCalib.y();
        const float fx = m_vFocal.x() * sx, fy = m_vFocal.y() * sy;

        // Pixel-centre convention matches OpenCV resizing of the calibrated image.
        const float cx = (m_vPrincipal.x() + 0.5f) * sx - 0.5f;
        const float cy = (m_vPrincipal.y() + 0.5f) * sy - 0.5f;
        cloud.reserve(((size_t(depth.cols) - 1) / m_nPCLstep + 1) *
                      ((size_t(depth.rows) - 1) / m_nPCLstep + 1));
        for (int y = 0; y < depth.rows; y += m_nPCLstep)
        {
            const float *row = depth.ptr<float>(y);
            for (int x = 0; x < depth.cols; x += m_nPCLstep)
            {
                const float z = row[x];
                if (!std::isfinite(z) || z <= 0)
                    continue;
                const float px = (x - cx) * z / fx, py = (y - cy) * z / fy;
                if (!std::isfinite(px) || !std::isfinite(py))
                    continue;
                const Vector3f point(px, py, z);
                Vector3f color(1, 1, 1);
                if (m_bPCLrgb)
                {
                    const Vec3b c = rgb.at<Vec3b>(y, x);
                    color = Vector3f(c[2] / 255.0f, c[1] / 255.0f, c[0] / 255.0f);
                }
                cloud.push_back({point, color, stamp});
            }
        }
    }

    void _YOLO26depthEstONNX::console(void *pConsole)
    {
        NULL_(pConsole);
        _DetectorBase::console(pConsole);
        if (m_pTpp)
            m_pTpp->console(pConsole);
    }
}
