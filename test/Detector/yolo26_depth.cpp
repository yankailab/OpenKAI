#include "Detector/_YOLO26depthEstONNX.h"
#include <atomic>
#include <cmath>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <thread>

using namespace kai;

namespace
{
    void require(bool condition, const string &message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    void near(float actual, float expected, const string &message)
    {
        require(std::isfinite(actual) && std::abs(actual - expected) < 1e-4f,
                message + ": expected " + std::to_string(expected) + ", got " + std::to_string(actual));
    }

    class DepthProbe : public _YOLO26depthEstONNX
    {
    public:
        using _YOLO26depthEstONNX::detect;
        using _YOLO26depthEstONNX::estimateDepth;
        using _YOLO26depthEstONNX::makePointCloud;
        using _YOLO26depthEstONNX::updatePCL;

        bool model(const string &filename)
        {
            m_fModel = filename;
            m_vModelInputSize = Vector2i(4, 4);
            m_vRangeD = Vector2f(0.001f, 1000.f);
            return loadModel();
        }

        void range(float low, float high) { m_vRangeD = Vector2f(low, high); }
        void correction(float scale, float offset) { m_dScale = scale; m_dOfs = offset; }
        void calibration(int width = 2, int height = 2)
        {
            m_vFocal = Vector2f(2.f, 4.f);
            m_vPrincipal = Vector2f(0.5f, 0.5f);
            m_vSizeCalib = Vector2i(width, height);
            m_nPCLstep = 1;
            m_bPCL = true;
            m_bPCLrgb = true;
        }
        void step(int value) { m_nPCLstep = value; }
        void color(bool enabled) { m_bPCLrgb = enabled; }
        bool pointWorkerRunning() const { return m_pTpp && m_pTpp->bRun(); }
        void joinInference() { if (m_pT) m_pT->join(); }
        void outputs(RGBframe &input, RGBDframe &rgbd, RGBframe &depth, PCLframe &points)
        {
            m_pRGBin = &input;
            m_pDout = &rgbd;
            m_pDepthOut = &depth;
            m_pPCLout = &points;
            if (!m_pT) m_pT = new _Thread();
        }
    };

    template <class Predicate>
    void waitUntil(Predicate ready, const string &message)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!ready())
        {
            require(std::chrono::steady_clock::now() < deadline, message);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    json workerSettings(const string &fixtures)
    {
        return {{"name", "depth"}, {"class", "_YOLO26depthEstONNX"},
                {"fModel", fixtures + "/channels_4d_static.onnx"}, {"nThread", 1},
                {"thread", {{"FPS", 1000}}}, {"threadPP", {{"FPS", 1}}},
                {"bPCL", true}, {"bPCLrgb", true}, {"nPCLstep", 1},
                {"vFocal", {2.f, 4.f}}, {"vPrincipal", {.5f, .5f}}, {"vSizeCalib", {4, 2}}};
    }

    class GatedDepthProbe : public DepthProbe
    {
    public:
        ~GatedDepthProbe() override
        {
            allow(std::numeric_limits<size_t>::max());
            stop(); // Join before destroying the gate used by the virtual callback.
        }

        void allow(size_t count)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_allowed = count;
            m_ready.notify_all();
        }

        uint64_t waitForCloud(size_t index) const
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            require(m_ready.wait_for(lock, std::chrono::seconds(3), [&] {
                return m_started.size() > index;
            }), "Point-cloud worker did not wake for the next completed depth frame");
            return m_started[index];
        }

        vector<uint64_t> started() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_started;
        }

    protected:
        void makePointCloud(const Mat &rgb, const Mat &depth, uint64_t stamp,
                            vector<GEOMETRY_POINT> &cloud) const override
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_started.push_back(stamp);
                const size_t count = m_started.size();
                m_ready.notify_all();
                m_ready.wait(lock, [&] { return m_allowed >= count; });
            }
            DepthProbe::makePointCloud(rgb, depth, stamp, cloud);
        }

    private:
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_ready;
        mutable vector<uint64_t> m_started;
        size_t m_allowed = 0;
    };

    void testInference(const string &fixtures)
    {
        // A 4x2 source becomes 4x4 with one padding row above and below. The
        // fixture computes 2 + R + 10G + 100B from the normalized NCHW input.
        Mat rgb(2, 4, CV_8UC3);
        for (int y = 0; y < rgb.rows; ++y)
            for (int x = 0; x < rgb.cols; ++x)
                rgb.at<Vec3b>(y, x) = Vec3b(51, 102, 51 * (x + 1));

        for (int rank : {2, 3, 4})
            for (const string shape : {"static", "dynamic"})
            {
                const string name = "channels_" + std::to_string(rank) + "d_" + shape;
                DepthProbe detector;
                require(detector.model(fixtures + "/" + name + ".onnx"), "Could not load " + name);
                Mat depth;
                require(detector.estimateDepth(rgb, depth), "Inference failed: " + name);
                require(depth.type() == CV_32FC1 && depth.size() == rgb.size(),
                        "Depth must be a full-resolution single-channel float image: " + name);
                for (int y = 0; y < depth.rows; ++y)
                    for (int x = 0; x < depth.cols; ++x)
                        near(depth.at<float>(y, x), 26.f + .2f * (x + 1),
                             "RGB normalization, channel order, metric output, or letterbox crop: " + name);

                Mat portrait(4, 2, CV_8UC3, Scalar(51, 102, 153));
                require(detector.estimateDepth(portrait, depth), "Portrait inference failed");
                require(depth.size() == portrait.size(), "Portrait output was not restored to source size");
                near(static_cast<float>(cv::mean(depth)[0]), 26.6f, "Horizontal letterbox crop failed");
                Mat thin(1, 4, CV_8UC3, Scalar(51, 102, 153));
                require(detector.estimateDepth(thin, depth), "Asymmetric padding inference failed");
                require(depth.size() == thin.size(), "Asymmetric padding changed source dimensions");
                near(static_cast<float>(cv::mean(depth)[0]), 26.6f, "Asymmetric letterbox crop failed");
                require(!detector.estimateDepth(Mat(), depth), "Empty input must be rejected");
            }
    }

    void testLetterboxHalfTie(const string &fixtures)
    {
        DepthProbe detector;
        require(detector.model(fixtures + "/channels_4d_dynamic.onnx"), "Could not load rounding fixture");
        Mat rgb(5, 8, CV_8UC3);
        for (int y = 0; y < rgb.rows; ++y)
            for (int x = 0; x < rgb.cols; ++x)
                rgb.at<Vec3b>(y, x) = Vec3b(10 * y * y, 3 * x * x, 17 * (x + y));

        // Python's round(5 * 0.5) is 2, so an 8x5 image uses 4x2 content
        // inside the 4x4 model input. Rounding up to 3 shifts the depth map.
        Mat resizedRGB, smallDepth(2, 4, CV_32FC1), expected, actual;
        cv::resize(rgb, resizedRGB, Size(4, 2), 0, 0, INTER_LINEAR);
        for (int y = 0; y < smallDepth.rows; ++y)
            for (int x = 0; x < smallDepth.cols; ++x)
            {
                const Vec3b pixel = resizedRGB.at<Vec3b>(y, x);
                smallDepth.at<float>(y, x) = 2.f + (pixel[2] + 10.f * pixel[1] + 100.f * pixel[0]) / 255.f;
            }
        cv::resize(smallDepth, expected, rgb.size(), 0, 0, INTER_LINEAR);
        require(detector.estimateDepth(rgb, actual), "Half-tie letterbox inference failed");
        require(actual.size() == expected.size(), "Half-tie letterbox changed output dimensions");
        require(norm(actual, expected, NORM_INF) < 1e-4,
                "Letterbox resize must use Python ties-to-even rounding for a 2.5-pixel side");
    }

    void testRangeAndInvalidDepth(const string &fixtures)
    {
        DepthProbe detector;
        require(detector.model(fixtures + "/invalid_depth.onnx"), "Could not load invalid-depth fixture");
        detector.range(.5f, 5.f);
        Mat rgb(2, 4, CV_8UC3, Scalar::all(0)), depth;
        require(detector.estimateDepth(rgb, depth), "Invalid-depth inference failed");
        const float expected[] = {0, 0, 0, 0, 0, 1, 3, 0};
        for (int i = 0; i < 8; ++i)
            near(depth.ptr<float>()[i], expected[i], "Non-finite/non-positive/out-of-range depth was not zeroed");

        detector.correction(2.f, 1.f);
        require(detector.estimateDepth(rgb, depth), "Corrected inference failed");
        for (int x = 0; x < 4; ++x)
            near(depth.at<float>(0, x), 0.f, "Positive offset must not revive invalid raw depth");
        near(depth.at<float>(1, 1), 3.f, "Metric scale and offset not applied");
        near(depth.at<float>(1, 2), 0.f, "Range was not applied after metric correction");

        DepthProbe wrong;
        if (wrong.model(fixtures + "/wrong_output.onnx"))
            require(!wrong.estimateDepth(rgb, depth), "A multi-channel output must be rejected");
    }

    void testReducedOutputAndPadding(const string &fixtures)
    {
        DepthProbe detector;
        require(detector.model(fixtures + "/reduced_depth.onnx"), "Could not load reduced output fixture");
        Mat rgb(2, 4, CV_8UC3, Scalar(51, 102, 153)), depth;
        require(detector.estimateDepth(rgb, depth), "Reduced output inference failed");
        for (int y = 0; y < depth.rows; ++y)
            for (int x = 0; x < depth.cols; ++x)
                near(depth.at<float>(y, x), 2.f + 2.f * x / 3.f + 4.f * (y + 1) / 3.f,
                     "Reduced output must use align_corners before removing padding");

        require(detector.model(fixtures + "/padding_mean.onnx"), "Could not load padding fixture");
        require(detector.estimateDepth(rgb, depth), "Padding mean inference failed");
        const float expected = (26.6f + 2.f + 111.f * 114.f / 255.f) / 2.f;
        near(depth.at<float>(0, 0), expected, "Centered letterbox border must be normalized gray 114");
    }

    void testPointCloud()
    {
        DepthProbe detector;
        detector.calibration();
        Mat rgb(2, 2, CV_8UC3);
        rgb.at<Vec3b>(0, 0) = Vec3b(51, 102, 153);
        rgb.at<Vec3b>(0, 1) = Vec3b(255, 0, 51);
        Mat depth(2, 2, CV_32FC1);
        depth.at<float>(0, 0) = 2.f;
        depth.at<float>(0, 1) = 4.f;
        depth.at<float>(1, 0) = 0.f;
        depth.at<float>(1, 1) = std::numeric_limits<float>::quiet_NaN();
        vector<GEOMETRY_POINT> points;
        detector.makePointCloud(rgb, depth, 123456789, points);
        require(points.size() == 2, "Point cloud must omit invalid depth");
        require(points[0].m_vP.isApprox(Vector3f(-.5f, -.25f, 2.f), 1e-6f),
                "First point does not match the calibrated optical frame");
        require(points[1].m_vP.isApprox(Vector3f(1.f, -.5f, 4.f), 1e-6f),
                "Second point does not match the calibrated optical frame");
        require(points[0].m_vC.isApprox(Vector3f(.6f, .4f, .2f), 1e-6f), "BGR point color must become RGB in [0,1]");
        require(points[1].m_vC.isApprox(Vector3f(.2f, 0.f, 1.f), 1e-6f), "Second point color is wrong");
        for (const auto &point : points)
            require(point.m_tStamp == 123456789, "Point lost the input capture timestamp");

        Mat enlargedRGB(4, 4, CV_8UC3, Scalar::all(0));
        Mat enlargedDepth(4, 4, CV_32FC1, Scalar(2.f));
        detector.makePointCloud(enlargedRGB, enlargedDepth, 234, points);
        require(points.size() == 16, "Rescaled camera cloud has the wrong size");
        require(points.front().m_vP.isApprox(Vector3f(-.75f, -.375f, 2.f), 1e-6f) &&
                    points.back().m_vP.isApprox(Vector3f(.75f, .375f, 2.f), 1e-6f),
                "Intrinsics must scale to output dimensions using pixel centers");

        detector.step(2);
        detector.makePointCloud(rgb, depth, 456, points);
        require(points.size() == 1 && points[0].m_tStamp == 456, "Point subsampling or cloud replacement failed");
        detector.color(false);
        detector.makePointCloud(rgb, depth, 789, points);
        require(points.size() == 1 && points[0].m_vC.isApprox(Vector3f::Ones()), "Uncolored points should be white like depth camera points");
    }

    void testFrameOutputs(const string &fixtures)
    {
        DepthProbe detector;
        require(detector.model(fixtures + "/channels_4d_static.onnx"), "Could not load output test fixture");
        detector.calibration(4, 2);
        RGBframe input, depthOutput;
        RGBDframe rgbdOutput;
        PCLframe pointOutput;
        detector.outputs(input, rgbdOutput, depthOutput, pointOutput);
        const Mat rgb(2, 4, CV_8UC3, Scalar(51, 102, 153));
        input.set(rgb, 100);
        detector.detect();
        detector.updatePCL();
        Mat color, depth, standaloneDepth;
        vector<GEOMETRY_POINT> points;
        require(rgbdOutput.get(color, depth) == 100, "RGBD timestamp must equal input timestamp");
        require(depthOutput.get(standaloneDepth) == 100, "Depth preview timestamp must equal input timestamp");
        require(pointOutput.get(points) == 100, "Point cloud timestamp must equal input timestamp");
        require(norm(rgb, color, NORM_INF) == 0, "RGBD output did not retain the input RGB image");
        require(norm(depth, standaloneDepth, NORM_INF) == 0, "Depth and RGBD outputs disagree");
        require(points.size() == rgb.total(), "Point cloud output is incomplete");
        near(depth.at<float>(0, 0), 26.6f, "Frame output is not metric depth");
        for (const auto &point : points) require(point.m_tStamp == 100, "Point timestamp differs from frame");

        // Change pixels while deliberately preserving the source timestamp.
        // Output must remain the previous frame until a fresh capture arrives.
        input.set(Mat(2, 4, CV_8UC3, Scalar::all(0)), 100);
        detector.detect();
        require(rgbdOutput.get(color, depth) == 100, "Repeated frame changed output timestamp");
        near(depth.at<float>(0, 0), 26.6f, "Repeated frame was processed twice");
        input.set(Mat(2, 4, CV_8UC3, Scalar::all(0)), 101);
        detector.detect();
        require(rgbdOutput.get(color, depth) == 101, "Fresh frame was not published");
        near(depth.at<float>(0, 0), 2.f, "Fresh frame inference did not update depth");
    }

    void testAsynchronousPointCloud(const string &fixtures)
    {
        JsonCfg owner;
        json settings = workerSettings(fixtures);
        RGBframe input, depthOutput;
        RGBDframe rgbdOutput;
        PCLframe pointOutput;
        GatedDepthProbe detector; // Destroy the worker before its output frames.
        detector.setConfig(&owner, &settings);
        require(detector.loadConfig(), "Could not configure asynchronous detector");
        detector.outputs(input, rgbdOutput, depthOutput, pointOutput);
        require(detector.start(), "Could not start native detector and point-cloud threads");
        require(detector.pointWorkerRunning(), "Point-cloud worker was not started");

        const Mat first(2, 4, CV_8UC3, Scalar(51, 102, 153));
        input.set(first, 201);
        require(detector.waitForCloud(0) == 201, "Point-cloud worker started with the wrong capture");
        require(rgbdOutput.getTstamp() == 201 && depthOutput.getTstamp() == 201,
                "Depth outputs must be published before point-cloud completion");
        require(pointOutput.getTstamp() == 0, "Blocked cloud should not already be published");

        // Keep the first cloud blocked while inference completes two more
        // captures. This proves overlap without relying on timing comparisons.
        input.set(Mat(2, 4, CV_8UC3, Scalar::all(0)), 202);
        waitUntil([&] { return depthOutput.getTstamp() == 202; },
                  "Inference waited for the blocked point-cloud worker");
        const Mat latest(2, 4, CV_8UC3, Scalar(5, 10, 15));
        input.set(latest, 203);
        waitUntil([&] { return depthOutput.getTstamp() == 203; },
                  "Inference did not replace the pending point-cloud capture");
        // Join only inference so its last detect() has completed the handoff.
        // The point-cloud worker remains blocked until explicitly released.
        detector.joinInference();

        detector.allow(1);
        require(detector.waitForCloud(1) == 203,
                "Busy point-cloud worker must process the newest pending capture");
        vector<GEOMETRY_POINT> points;
        require(pointOutput.get(points) == 201 && points.size() == first.total(),
                "First completed cloud lost its original capture");
        for (const auto &point : points)
        {
            require(point.m_tStamp == 201, "In-flight point timestamp was replaced");
            near(point.m_vP.z(), 26.6f, "In-flight cloud used another capture's depth");
            require(point.m_vC.isApprox(Vector3f(.6f, .4f, .2f), 1e-6f),
                    "In-flight cloud used another capture's RGB");
        }

        detector.allow(2);
        waitUntil([&] { return pointOutput.getTstamp() == 203; },
                  "Latest pending cloud was not published");
        require(pointOutput.get(points) == 203 && points.size() == latest.total(),
                "Latest cloud has the wrong timestamp or point count");
        for (const auto &point : points)
        {
            require(point.m_tStamp == 203, "Latest point timestamp differs from its frame");
            near(point.m_vP.z(), 2.f + 615.f / 255.f, "Latest cloud used stale depth");
            require(point.m_vC.isApprox(Vector3f(15.f, 10.f, 5.f) / 255.f, 1e-6f),
                    "Latest cloud used stale RGB");
        }
        detector.stop();
        require(detector.started() == vector<uint64_t>({201, 203}),
                "Point-cloud worker processed a stale pending capture or duplicate frame");
        require(!detector.pointWorkerRunning(), "Stopping the detector left the point-cloud worker active");
    }

    void testStopActiveWorker(const string &fixtures)
    {
        JsonCfg owner;
        json settings = workerSettings(fixtures);
        RGBframe input, depthOutput;
        RGBDframe rgbdOutput;
        PCLframe pointOutput;
        GatedDepthProbe detector;
        detector.setConfig(&owner, &settings);
        require(detector.loadConfig(), "Could not configure active worker shutdown test");
        detector.outputs(input, rgbdOutput, depthOutput, pointOutput);
        require(detector.start(), "Could not start active worker shutdown test");
        const Mat rgb(2, 4, CV_8UC3, Scalar(51, 102, 153));
        input.set(rgb, 401);
        require(detector.waitForCloud(0) == 401, "First cloud did not reach shutdown gate");
        input.set(rgb, 402);
        waitUntil([&] { return depthOutput.getTstamp() == 402; },
                  "Could not queue a second cloud before shutdown");
        detector.joinInference();

        std::atomic<bool> stopped{false};
        std::thread stopping([&] { detector.stop(); stopped = true; });
        bool returnedWhileBlocked;
        try
        {
            waitUntil([&] { return !detector.pointWorkerRunning(); },
                      "Stop did not request point-cloud shutdown");
            returnedWhileBlocked = stopped.load();
        }
        catch (...)
        {
            detector.allow(1);
            stopping.join();
            throw;
        }
        detector.allow(1);
        stopping.join();
        require(!returnedWhileBlocked, "Stop returned while point-cloud processing was still active");
        require(detector.started() == vector<uint64_t>({401}),
                "Stopping processed a pending cloud after the active cloud finished");

        detector.allow(2);
        input.set(rgb, 403);
        require(detector.start(), "Could not restart after stopping an active worker");
        waitUntil([&] { return pointOutput.getTstamp() == 403; },
                  "Restart after active shutdown did not publish the new cloud");
        detector.stop();
        require(detector.started() == vector<uint64_t>({401, 403}),
                "Restart reused a stale cloud pending before shutdown");
    }

    void testWorkerLifecycle(const string &fixtures)
    {
        JsonCfg owner;
        json settings = workerSettings(fixtures);
        RGBframe input, depthOutput;
        RGBDframe rgbdOutput;
        PCLframe pointOutput;
        DepthProbe detector;
        detector.setConfig(&owner, &settings);
        require(detector.loadConfig(), "Could not configure point-cloud lifecycle test");
        detector.outputs(input, rgbdOutput, depthOutput, pointOutput);
        require(detector.saveConfig(false) && settings["threadPP"]["FPS"] == 1,
                "Point-cloud thread configuration did not survive saving");
        const Mat rgb(2, 4, CV_8UC3, Scalar(51, 102, 153));

        // Exercise idle waiting, wakeup, joining and starting the same worker.
        for (uint64_t stamp = 301; stamp < 306; ++stamp)
        {
            require(detector.start(), "Stopped detector threads could not restart");
            input.set(rgb, stamp);
            waitUntil([&] { return pointOutput.getTstamp() == stamp; },
                      "Point-cloud worker lost a wakeup after startup or restart");
            detector.stop();
            detector.stop(); // An already stopped detector must be safe to stop again.
            require(!detector.pointWorkerRunning(), "Point-cloud worker did not join");
        }

        require(detector.start(), "Could not start before configuration reload");
        settings["nPCLstep"] = 2;
        require(detector.loadConfig(), "Live detector configuration reload failed");
        require(!detector.pointWorkerRunning(), "Reload must join the old point-cloud worker");
        input.set(rgb, 306);
        require(detector.start(), "Could not restart after configuration reload");
        waitUntil([&] { return pointOutput.getTstamp() == 306; },
                  "Reloaded point-cloud worker did not publish a frame");
        vector<GEOMETRY_POINT> points;
        require(pointOutput.get(points) == 306 && points.size() == 2,
                "Point-cloud worker did not use reloaded subsampling configuration");
        detector.stop();

        settings["bPCL"] = settings["bPCLrgb"] = false;
        require(detector.loadConfig(), "Depth-only reload failed");
        input.set(rgb, 307);
        require(detector.start(), "Depth-only detector did not start");
        waitUntil([&] { return depthOutput.getTstamp() == 307; },
                  "Depth-only detector failed to publish depth");
        require(!detector.pointWorkerRunning(), "Disabled point-cloud output started an unnecessary worker");
        detector.stop();
        require(pointOutput.getTstamp() == 306, "Depth-only detection changed the point-cloud output");
    }

    void testRealModel(const string &filename)
    {
        DepthProbe detector;
        require(detector.model(filename), "Could not load actual YOLO26 depth model: " + filename);
        detector.calibration(640, 480);
        RGBframe input, depthOutput;
        RGBDframe rgbdOutput;
        PCLframe pointOutput;
        detector.outputs(input, rgbdOutput, depthOutput, pointOutput);
        Mat rgb(480, 640, CV_8UC3);
        for (int y = 0; y < rgb.rows; ++y)
            for (int x = 0; x < rgb.cols; ++x)
                rgb.at<Vec3b>(y, x) = Vec3b(x % 256, y % 256, (x + y) % 256);
        input.set(rgb, 123456789);
        detector.detect();
        detector.updatePCL();
        Mat color, depth, standaloneDepth;
        vector<GEOMETRY_POINT> points;
        require(rgbdOutput.get(color, depth) == 123456789 &&
                    depthOutput.get(standaloneDepth) == 123456789 &&
                    pointOutput.get(points) == 123456789,
                "Actual model frame outputs lost input timestamp");
        require(depth.type() == CV_32FC1 && depth.size() == rgb.size() &&
                    cv::checkRange(depth, true, nullptr, 0, 1000),
                "Actual model depth has incorrect dimensions, type or non-finite values");
        require(norm(rgb, color, NORM_INF) == 0 && norm(depth, standaloneDepth, NORM_INF) == 0,
                "Actual model RGBD and depth outputs disagree");
        const auto validPixels = static_cast<size_t>(countNonZero(depth));
        require(validPixels > 0 && points.size() == validPixels, "Actual model point count differs from valid depth count");
        size_t index = 0;
        for (int y = 0; y < depth.rows; ++y)
            for (int x = 0; x < depth.cols; ++x)
            {
                const float z = depth.at<float>(y, x);
                if (z <= 0) continue;
                const auto &point = points[index++];
                require(point.m_tStamp == 123456789 && point.m_vP.allFinite(),
                        "Actual model emitted invalid point coordinates or timestamps");
                near(point.m_vP.z(), z, "Point Z differs from actual model depth");
                const Vec3b pixel = rgb.at<Vec3b>(y, x);
                require(point.m_vC.isApprox(Vector3f(pixel[2], pixel[1], pixel[0]) / 255.f, 1e-6f),
                        "Actual model point color differs from aligned RGB");
            }
        double low, high;
        minMaxLoc(depth, &low, &high);
        std::cout << "PASS: " << filename << ": 640x480 metric depth [" << low << ", " << high
                  << "], " << points.size() << " aligned RGB points, original timestamps\n";
    }

    class FrameManager : public InstanceMgr
    {
    public:
        template <class Frame> void add(const string &name)
        {
            auto *frame = new Frame;
            frame->setName(name);
            m_vDataStreams.push_back(frame);
        }
    };

    void testAutomaticThreadConfig(const string &fixtures)
    {
        JsonCfg owner;
        json settings = {{"name", "depth"}, {"class", "_YOLO26depthEstONNX"},
                         {"fModel", fixtures + "/channels_4d_static.onnx"}, {"nThread", 0}};
        DepthProbe detector;
        detector.setConfig(&owner, &settings);
        require(detector.loadConfig(), "Automatic ONNX CPU threading must load from configuration");
        require(detector.saveConfig(false) && settings["nThread"] == 0,
                "Automatic CPU threading must survive configuration save");
        Mat depth;
        require(detector.estimateDepth(Mat(2, 4, CV_8UC3, Scalar(51, 102, 153)), depth),
                "Automatic threading failed inference");
        near(depth.at<float>(0, 0), 26.6f, "CPU optimizations changed depth semantics");
        settings["nThread"] = -1;
        require(!detector.loadConfig(), "Negative CPU worker count must be rejected");
    }

    void testLinkWithoutBoundingBoxes()
    {
        JsonCfg owner;
        json settings = {{"name", "depth"}, {"class", "_YOLO26depthEstONNX"},
                         {"RGBframeIn", "rgb"}, {"RGBDframeOut", "rgbd"},
                         {"DframeOut", "depth"}, {"PCLframeOut", "points"}};
        FrameManager manager;
        manager.add<RGBframe>("rgb");
        manager.add<RGBDframe>("rgbd");
        manager.add<RGBframe>("depth");
        manager.add<PCLframe>("points");
        DepthProbe detector;
        detector.setConfig(&owner, &settings);
        require(detector.link(&manager), "Depth detector must link without a BBoxStreamOut");
        settings["RGBDframeOut"] = "rgb";
        require(!detector.link(&manager), "Wrong RGBD output type was accepted");
    }
}

int main(int argc, char **argv)
{
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = 2;
    try
    {
        if (argc >= 3 && string(argv[1]) == "--model")
        {
            for (int i = 2; i < argc; ++i) testRealModel(argv[i]);
            return 0;
        }
        require(argc == 2, "Expected fixture directory, or --model followed by ONNX paths");
        testInference(argv[1]);
        testLetterboxHalfTie(argv[1]);
        testRangeAndInvalidDepth(argv[1]);
        testReducedOutputAndPadding(argv[1]);
        testPointCloud();
        testFrameOutputs(argv[1]);
        testAsynchronousPointCloud(argv[1]);
        testStopActiveWorker(argv[1]);
        testWorkerLifecycle(argv[1]);
        testLinkWithoutBoundingBoxes();
        testAutomaticThreadConfig(argv[1]);
        std::cout << "PASS: ONNX inference/preprocessing, metric depth, invalids, point cloud geometry/colors, frame timestamps, asynchronous worker overlap/lifecycle and depth-only linking\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
