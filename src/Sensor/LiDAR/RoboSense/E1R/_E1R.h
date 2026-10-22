/* Native RoboSense E1R UDP driver. No RoboSense SDK is required. */
#ifndef OpenKAI_src_Sensor_LiDAR_RoboSense_E1R__E1R_H_
#define OpenKAI_src_Sensor_LiDAR_RoboSense_E1R__E1R_H_

#include "../../../../Universe/_ReferenceFrame.h"
#include "../../../../DataObject/PCLframe.h"
#include "E1RProtocol.h"
#include <mutex>

namespace kai
{
    class _E1R : public _ReferenceFrame
    {
        friend struct E1RTestAccess;
    public:
        _E1R();
        ~_E1R() override;
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
        static void *getUpdate(void *self) { static_cast<_E1R *>(self)->update(); return nullptr; }
        void receive(int socket, bool msop);
        void handleMSOP(const e1r::Msop &packet, uint64_t receivedNs);
        void handleDIFOP(const e1r::Difop &packet, uint64_t receivedNs);
        void publishFrame(bool partial = false);
        void resetFrame();

        mutable std::recursive_mutex m_mutex;
        PCLframe *m_pPCLout = nullptr;
        json m_config;
        json m_observed = json::object();
        int m_msopSocket = -1;
        int m_difopSocket = -1;
        uint64_t m_socketGeneration = 0;
        bool m_started = false;
        bool m_timedOut = false;
        string m_selectedIP;
        string m_lastError;
        uint64_t m_lastMSOP = 0;
        uint64_t m_lastDIFOP = 0;
        uint64_t m_lastPacketUs = 0;
        uint16_t m_lastSequence = 0;
        uint8_t m_timeMode = 0;
        bool m_haveSequence = false;
        vector<GEOMETRY_POINT> m_framePoints;
        uint64_t m_frameStamp = 0;
        uint64_t m_lastPublishedStamp = 0;
        bool m_frameActive = false;
        bool m_framePartial = false;
        uint64_t m_pointPackets = 0;
        uint64_t m_difopPackets = 0;
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
