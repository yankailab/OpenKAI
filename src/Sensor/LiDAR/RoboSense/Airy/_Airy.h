/* Native RoboSense Airy UDP driver. No RoboSense SDK is required. */
#ifndef OpenKAI_src_Sensor_LiDAR_RoboSense_Airy__Airy_H_
#define OpenKAI_src_Sensor_LiDAR_RoboSense_Airy__Airy_H_

#include "../../../../Universe/_TransformBase.h"
#include "../../../../DataObject/PCLframe.h"
#include "../../../../DataObject/IMUstream.h"
#include "../../../IMU/IMUpreview.h"
#include "AiryProtocol.h"
#include <mutex>

namespace kai
{
    class _Airy : public _TransformBase
    {
        friend struct AiryTestAccess;
    public:
        _Airy();
        ~_Airy() override;
        bool loadConfig() override;
        bool saveConfig(bool bExport) override;
        bool link(InstanceMgr *pM) override;
        bool check() override;
        bool start() override;
        void stop() override;
        void console(void *pConsole) override;
        void console(const json &request, void *pJSONbase) override;
        void clear();

    private:
        json configValues() const;
        json controlSchema() const;
        json statusValues() const;
        bool applyConfig(const json &patch, bool live, json &errors);
        bool openSockets(const json &config, string &error);
        void closeSockets();
        void update();
        static void *getUpdate(void *self) { static_cast<_Airy *>(self)->update(); return nullptr; }
        void receive(int socket, int kind);
        void handleMSOP(const airy::Msop &packet, uint64_t receivedNs);
        void handleDIFOP(const airy::Difop &packet, uint64_t receivedNs);
        void handleIMU(const airy::Imu &packet, uint64_t receivedNs);
        json imuValues() const;
        void resetIMUpreview();
        void resetSensor();
        void publishFrame(bool partial = false);
        void resetFrame();

        mutable std::recursive_mutex m_mutex;
        PCLframe *m_pPCLout = nullptr;
        IMUstream *m_pIMUout = nullptr;
        json m_config;
        json m_observed = json::object();
        int m_msopSocket = -1;
        int m_difopSocket = -1;
        int m_imuSocket = -1;
        uint64_t m_socketGeneration = 0;
        bool m_started = false;
        bool m_timedOut = false;
        string m_selectedIP;
        string m_lastError;
        uint64_t m_lastMSOP = 0;
        uint64_t m_lastDIFOP = 0;
        uint64_t m_lastIMU = 0;
        uint64_t m_lastIMUstamp = 0;
        uint64_t m_lastPacketNs = 0;
        uint32_t m_lastSequence = 0;
        bool m_haveSequence = false;
        bool m_havePacket = false;
        bool m_haveAzimuth = false;
        uint16_t m_lastAzimuth = 0;
        airy::Calibration m_calibration;
        airy::Imu m_latestIMU;
        // Viewer-only fusion, independent of raw IMUstream and cloud transforms.
        bool m_bIMUpreview = false;
        IMUpreview m_imuPreview;
        Quaterniond m_imuLeveling = Quaterniond::Identity();
        uint64_t m_previewStamp = 0;
        bool m_haveIMUleveling = false;
        vector<GEOMETRY_POINT> m_framePoints;
        uint64_t m_frameStamp = 0;
        uint64_t m_lastPublishedStamp = 0;
        bool m_frameActive = false;
        bool m_framePartial = false;
        uint64_t m_pointPackets = 0;
        uint64_t m_difopPackets = 0;
        uint64_t m_imuPackets = 0;
        uint64_t m_imuSamples = 0;
        uint64_t m_uncalibratedPackets = 0;
        uint64_t m_imuOutOfOrder = 0;
        uint64_t m_points = 0;
        uint64_t m_frames = 0;
        uint64_t m_partialFrames = 0;
        uint64_t m_droppedPackets = 0;
        uint64_t m_invalidPackets = 0;
        uint64_t m_outOfOrderPackets = 0;
        uint64_t m_frameOverflows = 0;
        uint64_t m_clockResets = 0;
    };
}
#endif
