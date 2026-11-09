#ifndef OPENKAI_SENSOR_LIVOX1_H
#define OPENKAI_SENSOR_LIVOX1_H

#include "../../../../Universe/_TransformRigid.h"
#include "../../../../DataObject/PCLframe.h"
#include "../../../../DataObject/IMUstream.h"
#include "../../../../Utility/util.h"
#include "../../../../Utility/utilNet.h"
#include "../../../IMU/IMUpreview.h"
#include "Livox1Protocol.h"
#include <array>
#include <map>
#include <mutex>

namespace kai
{
    struct LVX1_CONFIG
    {
        uint32_t m_hostIP = 0;
        uint16_t m_hostPortPCL = 56001, m_hostPortIMU = 56002;
        uint8_t m_coordinate = 0, m_returnMode = 0, m_scanPattern = 0, m_workMode = 1, m_imuDataEn = 1;
        bool m_sampling = true, m_fan = true, m_highSensitivity = true;
    };

    class _Livox1 : public _ModuleBase
    {
        friend struct Livox1TestAccess;
    public:
        _Livox1();
        ~_Livox1();
        bool loadConfig() override;
        bool saveConfig(bool bExport) override;
        bool link(InstanceMgr *manager) override;
        bool check() override;
        bool start() override;
        void stop() override;
        virtual void clear();
        void console(void *console) override;
        void jsonCmd(const json &request, void *endpoint) override;
        LVX1_CONFIG getConfig();
        void setConfig(const LVX1_CONFIG &config);

    private:
        enum SocketChannel { Broadcast, Command, PointCloud, Imu, SocketCount };
        enum class State { Discovering, Handshaking, Initializing, Configuring, Connected };
        struct PendingCommand
        {
            vector<uint8_t> bytes;
            uint64_t sent = 0;
            unsigned attempts = 1;
            bool setting = false, switching = false;
            string key;
        };
        bool openSockets(string &error);
        void closeSockets();
        bool sendDatagram(const vector<uint8_t> &bytes);
        void update();
        static void *getUpdate(void *self) { static_cast<_Livox1 *>(self)->update(); return nullptr; }
        void receive(size_t channel);
        void service(uint64_t now);
        void reconnect(const string &reason);
        bool recvLivoxCmd(const uint8_t *bytes, size_t length);
        bool recvLivoxData(const uint8_t *bytes, size_t length);
        void handleBroadcast(const uint8_t *bytes, size_t length);
        void handleCtrlCmdAck(const uint8_t *bytes, size_t length);
        void sendCommand(uint8_t set, uint8_t id, const vector<uint8_t> &payload = {}, bool setting = false, const string &key = "");
        void sendHandshake();
        void configureDevice();
        void finishConfiguration();
        void setLvxValue(const string &key);
        void querySetting(const string &key);
        bool supportsSetting(const string &key) const;
        void handlePointCloudData(const uint8_t *bytes, size_t length);
        void handleIMUdata(const uint8_t *bytes, size_t length);
        uint64_t dataTimestamp(const uint8_t *bytes);
        void resetFrame();
        void resetClocks();
        json configValues() const;
        json controlSchema() const;
        json imuValues() const;
        bool applyConfig(const json &patch, bool live, json &errors);

        _TransformRigid *m_pTf = nullptr;
        PCLframe *m_pPCLout = nullptr;
        IMUstream *m_pIMUout = nullptr;
        mutable std::recursive_mutex m_mutex;
        std::array<int, SocketCount> m_sockets{{-1, -1, -1, -1}};
        bool m_started = false;
        State m_state = State::Discovering;
        LVX1_CONFIG m_lvxCfg;
        string m_lvxSN, m_detectedSN;
        uint32_t m_lvxIP = 0, m_hostIP = 0;
        uint16_t m_broadcastPort = 55000, m_lvxCmdPort = 65000, m_hostPortCmd = 56000;
        int m_receiveBufferBytes = 4194304;
        uint8_t m_lvxDevType = livox1::Unknown, m_workState = 0;
        std::array<uint8_t, 4> m_firmware{};
        bool m_firmwareKnown = false, m_configFailed = false, m_samplingSent = false;
        uint16_t m_sequence = 0;
        std::map<uint16_t, PendingCommand> m_pending;
        uint64_t m_timeoutNs = 10 * NSEC_SEC, m_lastRx = 0, m_lastHeartbeat = 0, m_lastHandshake = 0;
        string m_lastError;
        json m_observed = json::object();

        Quaterniond m_mountOrientation = Quaterniond::Identity();
        bool m_bIMUpreview = false, m_bIMUstab = false;
        IMUpreview m_imuPreview;
        uint64_t m_tIMU = 0, m_lastIMUrx = 0;
        uint8_t m_timeType = 255;
        uint64_t m_ppsAnchorHost = 0;
        int64_t m_ppsAnchorOffset = 0;
        vector<GEOMETRY_POINT> m_vFramePoints;
        Vector3f m_vColorDefault = Vector3f::Ones();
        Vector2f m_vRangeD = Vector2f(0.1f, 500.0f);
        bool m_bReflectivity = true, m_bPCLframe = false;
        int m_nMaxFramePoints = 100000;
        uint64_t m_frameDurationNs = 100 * NSEC_MSEC, m_tPCLframe = 0, m_tLastPCL = 0;
        uint64_t m_pointPackets = 0, m_points = 0, m_frames = 0, m_imuSamples = 0,
            m_invalidPackets = 0, m_outOfOrderPackets = 0, m_frameOverflows = 0,
            m_commandErrors = 0, m_commandRetries = 0;
    };
}
#endif
