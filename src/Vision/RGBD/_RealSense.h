/* RealSense SDK 2 camera, motion streams and runtime controls. */
#ifndef OpenKAI_src_Vision_RGBD__RealSense_H_
#define OpenKAI_src_Vision_RGBD__RealSense_H_

#include <librealsense2/rs.hpp>
#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include "_RGBDbase.h"
#include "RealSenseIMU.h"

namespace kai
{
    class _RealSense : public _RGBDbase
    {
    public:
        _RealSense();
        ~_RealSense() override;
        bool loadConfig(void) override;
        bool saveConfig(bool bExport) override;
        bool start(void) override;
        void stop(void) override;
        bool open(void) override;
        void close(void) override;
        void console(void *pConsole) override;
        void console(const json &j, void *pJSONbase) override;

    private:
        std::unique_lock<std::recursive_mutex> lockDeviceForWork() const;
        json configValues() const;
        json controlSchema();
        json imuValues() const;
        bool applyConfig(const json &patch, bool live, json &errors);
        void setConfigValues(const json &values);
        void discoverOptions();
        void applyOptions();
        std::map<string, rs2::options> optionTargets();
        void receiveFrame(rs2::frame frame);
        void processFrames(rs2::frameset frames);
        void updatePC(const rs2::frame &depth, const rs2::frame &color, const Mat &rgb, uint64_t stamp);
        void update();
        void updateTPP();
        static void *getUpdate(void *self) { static_cast<_RealSense *>(self)->update(); return nullptr; }
        static void *getTPP(void *self) { static_cast<_RealSense *>(self)->updateTPP(); return nullptr; }

        mutable std::mutex m_deviceAdmission;
        mutable std::recursive_mutex m_deviceMutex;
        string m_SN;
        int m_accelFPS = 0;
        int m_gyroFPS = 0;
        int m_tOutMs = 1000;
        bool m_bAlign = false;
        bool m_bDecimation = false;
        bool m_bSpatial = false;
        bool m_bTemporal = false;
        bool m_bHoleFilling = false;
        bool m_bThreshold = false;
        bool m_pipelineStarted = false;
        json m_sensorOptions = json::object();
        std::set<string> m_preStreamWritable;
        string m_lastError;
        rs2::pipeline m_pipeline;
        rs2::pipeline_profile m_profile;
        std::map<string, rs2::sensor> m_sensors;
        rs2::frame_queue m_frames{1};
        rs2::decimation_filter m_decimation;
        rs2::spatial_filter m_spatial;
        rs2::temporal_filter m_temporal;
        rs2::hole_filling_filter m_holeFilling;
        rs2::threshold_filter m_threshold;
        rs2::align m_align{RS2_STREAM_COLOR};
        rs2::pointcloud m_pointcloud;
        realsense::IMUPreview m_imuPreview;
        std::atomic<uint64_t> m_lastVideo{0};
        std::atomic<uint64_t> m_lastAccel{0};
        std::atomic<uint64_t> m_lastGyro{0};
        std::atomic<uint64_t> m_nVideo{0};
        std::atomic<uint64_t> m_nAccel{0};
        std::atomic<uint64_t> m_nGyro{0};
    };
}
#endif
