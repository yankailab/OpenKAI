/*
 * _YOLO26depthEstONNX.h
 * Monocular metric depth and RGB point clouds from YOLO26 ONNX models.
 */

#ifndef OpenKAI_src_Detector__YOLO26depthEstONNX_H_
#define OpenKAI_src_Detector__YOLO26depthEstONNX_H_

#include "_DetectorBase.h"
#include "../DataObject/RGBDframe.h"
#include "../DataObject/PCLframe.h"
#include <onnxruntime_cxx_api.h>
#include <memory>
#include <condition_variable>
#include <mutex>

namespace kai
{
    class _YOLO26depthEstONNX : public _DetectorBase
    {
    public:
        _YOLO26depthEstONNX();
        ~_YOLO26depthEstONNX() override;

        bool loadConfig(void) override;
        bool saveConfig(bool bExport) override;
        bool link(InstanceMgr *pM) override;
        bool start(void) override;
        void stop(void) override;
        bool check(void) override;
        bool loadModel(void) override;
        void console(void *pConsole) override;

    protected:
        bool estimateDepth(const Mat &input, Mat &depth);
        virtual void makePointCloud(const Mat &rgb, const Mat &depth, uint64_t stamp,
                            vector<GEOMETRY_POINT> &cloud) const;
        void detect(void);
        void queuePointCloud(const Mat &rgb, const Mat &depth, uint64_t stamp);
        void updatePCL(void);
        _Thread *getThread(const string &name) override;

    private:
        void update(void) override;
        static void *getUpdate(void *This)
        {
            static_cast<_YOLO26depthEstONNX *>(This)->update();
            return nullptr;
        }

        // One pending snapshot; the worker owns its current job independently.
        std::mutex m_mtxPCL;
        std::condition_variable m_cvPCL;
        Mat m_pendingRGB;
        Mat m_pendingDepth;
        uint64_t m_pendingStamp = 0;

        void updateTPP(void);
        static void *getTPP(void *This)
        {
            static_cast<_YOLO26depthEstONNX *>(This)->updateTPP();
            return nullptr;
        }

    protected:
        // Depth is aligned to the input RGB, CV_32FC1 in metres; zero is invalid.
        RGBDframe *m_pDout = nullptr;
        RGBframe *m_pDepthOut = nullptr; // Optional depth-only output for _D2RGB.
        PCLframe *m_pPCLout = nullptr;

        _Thread *m_pTpp = nullptr;

        Ort::Env m_env;
        Ort::SessionOptions m_sessionOptions;
        Ort::MemoryInfo m_memoryInfo;
        std::unique_ptr<Ort::Session> m_pSession;
        string m_inputName;
        string m_outputName;
        Vector2i m_vModelInputSize = Vector2i(768, 768);
        bool m_bSwapRB = true;
        float m_scale = 1.0f / 255.0f;
        int m_nThread = 0; // 0 lets ONNX Runtime select its physical-core worker pool.

        Vector2f m_vRangeD = Vector2f(0, FLT_MAX);
        float m_dScale = 1.0f;
        float m_dOfs = 0.0f;
        bool m_bPCL = false;
        bool m_bPCLrgb = false;
        // Pinhole intrinsics in pixels at vSizeCalib, for an undistorted RGB image.
        Vector2f m_vFocal = Vector2f::Zero();
        Vector2f m_vPrincipal = Vector2f::Zero();
        Vector2i m_vSizeCalib = Vector2i::Zero();
        int m_nPCLstep = 1;

    };
}
#endif
