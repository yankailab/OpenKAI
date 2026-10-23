/* Livox SDK2 Ethernet driver. Mid-360 uses non-repetitive, timestamped scans. */
#include "_Livox2.h"
#include "../../../Protocol/_JSONbase.h"

namespace kai
{
    namespace
    {
        string ipString(uint32_t address)
        {
            const auto *p = reinterpret_cast<const uint8_t *>(&address);
            return std::to_string(p[0]) + "." + std::to_string(p[1]) + "." + std::to_string(p[2]) + "." + std::to_string(p[3]);
        }
        bool readIP(const json &value, uint32_t &address)
        {
            return value.is_string() && inet_pton(AF_INET, value.get<string>().c_str(), &address) == 1;
        }
    }

    _Livox2::_Livox2() { m_lvxCfg.init(); }
    _Livox2::~_Livox2()
    {
        stop();
        DEL(m_pTdeviceQueryR); DEL(m_pTctrlCmdW); DEL(m_pTctrlCmdR);
        DEL(m_pTpushCmdR); DEL(m_pTpclR); DEL(m_pTimuR);
    }

    void _Livox2::stop()
    {
        const std::initializer_list<_Thread *> workers = {m_pT, m_pTdeviceQueryR, m_pTctrlCmdW, m_pTctrlCmdR,
            m_pTpushCmdR, m_pTpclR, m_pTimuR};
        for (auto *worker : workers) if (worker) worker->stop();
        for (auto *worker : workers) if (worker) worker->join();
    }

    bool _Livox2::loadConfig()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        IF_F(!_TransformBase::loadConfig());
        m_mountOrientation = m_vOrt;
        json patch = json::object(), errors;
        const json values = configValues();
        for (const auto &entry : values.items())
            if (m_pJ->contains(entry.key())) patch[entry.key()] = (*m_pJ)[entry.key()];
        if (!applyConfig(patch, false, errors)) { LOG_E(errors.dump()); return false; }
        const std::pair<_Thread **, const char *> workers[] = {
            {&m_pTdeviceQueryR, "TdeviceQueryR"}, {&m_pTctrlCmdW, "TctrlCmdW"}, {&m_pTctrlCmdR, "TctrlCmdR"},
            {&m_pTpushCmdR, "TpushCmdR"}, {&m_pTpclR, "TpclR"}, {&m_pTimuR, "TimuR"}};
        for (const auto &worker : workers)
        {
            delete *worker.first;
            *worker.first = createThread(jK(*m_pJ, worker.second), worker.second);
            if (!*worker.first) return false;
        }
        m_vFramePoints.reserve(m_nMaxFramePoints);
        return true;
    }

    bool _Livox2::saveConfig(bool bExport)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        IF_F(!_TransformBase::saveConfig(false));
        m_pJ->update(configValues());
        (*m_pJ)["vOrt"] = {m_mountOrientation.x(), m_mountOrientation.y(), m_mountOrientation.z(), m_mountOrientation.w()};
        for (auto *worker : {m_pTdeviceQueryR, m_pTctrlCmdW, m_pTctrlCmdR, m_pTpushCmdR, m_pTpclR, m_pTimuR})
            if (worker && !worker->saveConfig(false)) return false;
        return !bExport || m_pJcfg->saveToFile();
    }

    bool _Livox2::link(InstanceMgr *pM)
    {
        IF_F(!this->_TransformBase::link(pM));
        const json &j = *m_pJ;

        string n;

        n = "";
        jKv(j, "PCLframeOut", n);
        m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pPCLout, "PCLframeOut not found: " + n);

        n = "";
        jKv(j, "IMUstreamOut", n);
        m_pIMUout = dynamic_cast<IMUstream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!n.empty() && !m_pIMUout, "IMUstreamOut not found: " + n);

        n = "";
        jKv(j, "BytePacketStreamDeviceQueryIn", n);
        m_pBpStreamDeviceQueryIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pBpStreamDeviceQueryIn, "BytePacketStreamDeviceQueryIn not found: " + n);

        n = "";
        jKv(j, "BytePacketStreamDeviceQueryOut", n);
        m_pBpStreamDeviceQueryOut = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pBpStreamDeviceQueryOut, "BytePacketStreamDeviceQueryOut not found: " + n);

        n = "";
        jKv(j, "BytePacketStreamCtrlCmdIn", n);
        m_pBpStreamCtrlCmdIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pBpStreamCtrlCmdIn, "BytePacketStreamCtrlCmdIn not found: " + n);

        n = "";
        jKv(j, "BytePacketStreamCtrlCmdOut", n);
        m_pBpStreamCtrlCmdOut = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pBpStreamCtrlCmdOut, "BytePacketStreamCtrlCmdOut not found: " + n);

        n = "";
        jKv(j, "BytePacketStreamPushCmdIn", n);
        m_pBpStreamPushCmdIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pBpStreamPushCmdIn, "BytePacketStreamPushCmdIn not found: " + n);

        n = "";
        jKv(j, "BytePacketStreamPclIn", n);
        m_pBpStreamPclIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pBpStreamPclIn, "BytePacketStreamPclIn not found: " + n);

        n = "";
        jKv(j, "BytePacketStreamImuIn", n);
        m_pBpStreamImuIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pBpStreamImuIn, "BytePacketStreamImuIn not found: " + n);

        return true;
    }

    bool _Livox2::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTdeviceQueryR);
        NULL_F(m_pTctrlCmdW);
        NULL_F(m_pTctrlCmdR);
        NULL_F(m_pTpushCmdR);
        NULL_F(m_pTpclR);
        NULL_F(m_pTimuR);

        if (!m_pT->startThread(getUpdateWdeviceQuery, this)) { stop(); return false; }
        if (!m_pTdeviceQueryR->startThread(getUpdateRdeviceQuery, this)) { stop(); return false; }
        if (!m_pTctrlCmdW->startThread(getUpdateWctrlCmd, this)) { stop(); return false; }
        if (!m_pTctrlCmdR->startThread(getUpdateRctrlCmd, this)) { stop(); return false; }
        if (!m_pTpushCmdR->startThread(getUpdateRpushCmd, this)) { stop(); return false; }
        if (!m_pTpclR->startThread(getUpdateRpointCloud, this)) { stop(); return false; }
        if (!m_pTimuR->startThread(getUpdateRimu, this)) { stop(); return false; }

        return true;
    }

    bool _Livox2::check(void)
    {
        NULL_F(m_pPCLout);
        NULL_F(m_pBpStreamDeviceQueryIn);
        NULL_F(m_pBpStreamDeviceQueryOut);
        NULL_F(m_pBpStreamCtrlCmdIn);
        NULL_F(m_pBpStreamCtrlCmdOut);
        NULL_F(m_pBpStreamPushCmdIn);
        NULL_F(m_pBpStreamPclIn);
        NULL_F(m_pBpStreamImuIn);

        return this->_TransformBase::check();
    }

    void _Livox2::updateRdeviceQuery(void)
    {
        while (m_pTdeviceQueryR->bRun())
        {
            m_pTdeviceQueryR->autoFPS();

            vector<BYTE_PACKET> vPacket;
            m_pBpStreamDeviceQueryIn->get(vPacket, m_tLastBpStreamDeviceQueryIn);
            for (const BYTE_PACKET &packet : vPacket)
            {
                m_tLastBpStreamDeviceQueryIn = packet.m_tStamp;
                LIVOX2_CMD data;
                if (recvLivoxCmd(packet, &data))
                {
                    handleDeviceQuery(data);
                }
            }
        }
    }

    void _Livox2::updateRctrlCmd(void)
    {
        while (m_pTctrlCmdR->bRun())
        {
            m_pTctrlCmdR->autoFPS();

            vector<BYTE_PACKET> vPacket;
            m_pBpStreamCtrlCmdIn->get(vPacket, m_tLastBpStreamCtrlCmdIn);
            for (const BYTE_PACKET &packet : vPacket)
            {
                m_tLastBpStreamCtrlCmdIn = packet.m_tStamp;
                LIVOX2_CMD data;
                if (recvLivoxCmd(packet, &data))
                {
                    handleCtrlCmdAck(data);
                }
            }
        }
    }

    void _Livox2::updateRpushCmd(void)
    {
        while (m_pTpushCmdR->bRun())
        {
            m_pTpushCmdR->autoFPS();

            vector<BYTE_PACKET> vPacket;
            m_pBpStreamPushCmdIn->get(vPacket, m_tLastBpStreamPushCmdIn);
            for (const BYTE_PACKET &packet : vPacket)
            {
                m_tLastBpStreamPushCmdIn = packet.m_tStamp;
                LIVOX2_CMD data;
                if (recvLivoxCmd(packet, &data))
                {
                    handlePushCmd(data);
                }
            }
        }
    }

    void _Livox2::updateRpointCloud(void)
    {
        while (m_pTpclR->bRun())
        {
            m_pTpclR->autoFPS();

            vector<BYTE_PACKET> vPacket;
            m_pBpStreamPclIn->get(vPacket, m_tLastBpStreamPclIn);
            for (const BYTE_PACKET &packet : vPacket)
            {
                m_tLastBpStreamPclIn = packet.m_tStamp;
                LIVOX2_DATA data;
                if (recvLivoxData(packet, &data))
                {
                    handlePointCloudData(data);
                }
            }
        }
    }

    void _Livox2::updateRimu(void)
    {
        while (m_pTimuR->bRun())
        {
            m_pTimuR->autoFPS();

            vector<BYTE_PACKET> vPacket;
            m_pBpStreamImuIn->get(vPacket, m_tLastBpStreamImuIn);
            for (const BYTE_PACKET &packet : vPacket)
            {
                m_tLastBpStreamImuIn = packet.m_tStamp;
                LIVOX2_DATA data;
                if (recvLivoxData(packet, &data))
                {
                    handleIMUdata(data);
                }
            }
        }
    }

    bool _Livox2::recvLivoxCmd(const BYTE_PACKET &packet, LIVOX2_CMD *cmd, bool parity)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (!cmd || !livox2::validCommand(packet.m_vB.data(), packet.m_vB.size(), parity))
        { ++m_invalidPackets; return false; }
        static_assert(offsetof(LIVOX2_CMD, data) == LVX2_CMD_N_HDR, "Livox command layout");
        memset(cmd, 0, sizeof(*cmd));
        memcpy(cmd, packet.m_vB.data(), packet.m_vB.size());
        return true;
    }

    bool _Livox2::recvLivoxData(const BYTE_PACKET &packet, LIVOX2_DATA *data, bool parity)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (!data || !livox2::validData(packet.m_vB.data(), packet.m_vB.size(), parity))
        { ++m_invalidPackets; return false; }
        const uint8_t *p = packet.m_vB.data();
        data->version = p[0]; data->length = livox2::u16(p + 1);
        data->time_interval = livox2::u16(p + 3); data->dot_num = livox2::u16(p + 5);
        data->udp_cnt = livox2::u16(p + 7); data->frame_cnt = p[9];
        data->data_type = p[10]; data->time_type = p[11];
        memcpy(data->rsvd, p + 12, 12); data->crc32 = livox2::u32(p + 24);
        memcpy(data->timestamp, p + 28, 8);
        memcpy(data->data, p + 36, data->length - 36);
        return true;
    }

    void _Livox2::updateWdeviceQuery()
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            const uint64_t now = getTns();
            if (m_lvxState != lvxState_deviceQuery && m_lastRx && now - m_lastRx > m_timeoutNs)
            {
                m_lvxState = lvxState_deviceQuery;
                if (m_pPCLout) m_pPCLout->set({}, m_tLastPCL ? m_tLastPCL : 1);
                m_pending.clear(); m_configFailed = false;
                m_lastError = "LiDAR timed out; rediscovering";
                resetFrame(); m_imuPreview.reset(); m_tIMU = 0;
                if (m_pIMUout) m_pIMUout->clear();
            }
            if (m_lvxState == lvxState_deviceQuery) sendDeviceQuery();
        }
    }

    void _Livox2::sendDeviceQuery()
    {
        if (!m_pBpStreamDeviceQueryOut) return;
        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_DISCOVER, LVX2_CMD_REQ, ++m_sequence); cmd.calcCRC();
        m_pBpStreamDeviceQueryOut->add({{vector<uint8_t>((uint8_t *)&cmd, (uint8_t *)&cmd + cmd.length), getTns()}});
    }

    void _Livox2::handleDeviceQuery(const LIVOX2_CMD &cmd)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (cmd.cmd_id != LVX2_CMD_DISCOVER || cmd.cmd_type != LVX2_CMD_ACK || cmd.length < 48 || cmd.data[0]) return;
        uint32_t address;
        memcpy(&address, cmd.data + 18, 4);
        if (m_lvxIP && address != m_lvxIP) return;
        const string serial(reinterpret_cast<const char *>(cmd.data + 2), strnlen(reinterpret_cast<const char *>(cmd.data + 2), 16));
        if (!m_lvxSN.empty() && serial != m_lvxSN) return;
        m_detectedSN = serial;
        m_lvxCmdPort = livox2::u16(cmd.data + 22);
        m_lvxDevType = cmd.data[1];
        m_lastRx = getTns();
        if (m_lvxState != lvxState_deviceQuery) return;
        // The UDP command endpoint is configured independently, as with the
        // other packet-stream drivers. Mid-360 advertises port 56100.
        if (m_lvxCmdPort != 56100)
        {
            m_lastError = "Unsupported LiDAR command port " + std::to_string(m_lvxCmdPort) + "; configure the UDP command endpoint";
            return;
        }
        m_lvxState = lvxState_init;
        m_pending.clear(); m_configFailed = false; m_lastError.clear();
        resetFrame(); m_imuPreview.reset(); m_tIMU = 0;
        if (m_pIMUout) m_pIMUout->clear();
        configureDevice();
    }

    void _Livox2::sendCommand(LIVOX2_CMD cmd)
    {
        if (!m_pBpStreamCtrlCmdOut) return;
        cmd.seq_num = ++m_sequence; cmd.calcCRC();
        const uint64_t now = getTns();
        m_pending.emplace(cmd.seq_num, PendingCommand{cmd, now, 1});
        m_pBpStreamCtrlCmdOut->add({{vector<uint8_t>((uint8_t *)&cmd, (uint8_t *)&cmd + cmd.length), now}});
    }

    void _Livox2::setLvxByte(uint16_t key, uint8_t value)
    {
        // A newer browser edit supersedes retries of an older value.
        for (auto it = m_pending.begin(); it != m_pending.end();)
        {
            const auto &old = it->second.command;
            if (old.cmd_id == LVX2_CMD_SET && old.length == 33 && livox2::u16(old.data + 4) == key)
                it = m_pending.erase(it);
            else ++it;
        }
        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        cmd.addData(uint16_t(1)); cmd.addData(uint16_t(0));
        cmd.addData(key); cmd.addData(uint16_t(1)); cmd.addData(value);
        sendCommand(cmd);
    }

    void _Livox2::setLvxHost()
    {
        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        cmd.addData(uint16_t(3)); cmd.addData(uint16_t(0));
        const uint16_t keys[] = {kKeyStateInfoHostIpCfg, kKeyLidarPointDataHostIpCfg, kKeyLidarImuHostIpCfg};
        const uint16_t ports[] = {m_lvxCfg.m_hostPortState, m_lvxCfg.m_hostPortPCL, m_lvxCfg.m_hostPortIMU};
        const uint16_t sourcePorts[] = {56200, 56300, 56400};
        for (int i = 0; i < 3; ++i)
        {
            cmd.addData(keys[i]); cmd.addData(uint16_t(8)); cmd.addData(&m_lvxCfg.m_hostIP, 4);
            cmd.addData(ports[i]); cmd.addData(sourcePorts[i]);
        }
        sendCommand(cmd);
    }

    void _Livox2::configureDevice()
    {
        setLvxHost();
        setLvxByte(kKeyPclDataType, m_lvxCfg.m_pclDataType);
        setLvxByte(kKeyPatternMode, m_lvxCfg.m_patternMode);
        setLvxByte(kKeyDetectMode, m_lvxCfg.m_detectMode);
        setLvxByte(kKeyImuDataEn, m_lvxCfg.m_imuDataEn);
        setLvxByte(kKeyWorkMode, m_lvxCfg.m_workMode);
    }

    void _Livox2::updateWctrlCmd()
    {
        while (m_pTctrlCmdW->bRun())
        {
            m_pTctrlCmdW->autoFPS();
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            if (m_lvxState == lvxState_deviceQuery) continue;
            const uint64_t now = getTns();
            for (auto it = m_pending.begin(); it != m_pending.end();)
            {
                auto &pending = it->second;
                if (now - pending.sent < 500 * NSEC_MSEC) { ++it; continue; }
                if (pending.attempts >= 3)
                {
                    ++m_commandErrors;
                    if (pending.command.cmd_id == LVX2_CMD_SET) m_configFailed = true;
                    m_lastError = "LiDAR command " + std::to_string(pending.command.cmd_id) + " timed out after 3 attempts";
                    it = m_pending.erase(it); continue;
                }
                ++pending.attempts; ++m_commandRetries; pending.sent = now;
                const auto *bytes = reinterpret_cast<const uint8_t *>(&pending.command);
                m_pBpStreamCtrlCmdOut->add({{vector<uint8_t>(bytes, bytes + pending.command.length), now}});
                ++it;
            }
            if (m_pending.empty() && now - m_lastQuery >= 2 * NSEC_SEC)
            {
                getLvxConfig(); m_lastQuery = now;
            }
        }
    }

    void _Livox2::getLvxConfig()
    {
        // Mid-360 rejects an entire GET containing unsupported HAP-only keys
        // (notably frame rate, boot mode and point-send enable).
        const uint16_t keys[] = {kKeyPclDataType, kKeyPatternMode, kKeyStateInfoHostIpCfg,
            kKeyLidarPointDataHostIpCfg, kKeyLidarImuHostIpCfg, kKeyDetectMode, kKeyWorkMode, kKeyImuDataEn,
            kKeyCurWorkState, kKeyLidarDiagStatus};
        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_GET, LVX2_CMD_REQ, 0);
        cmd.addData(uint16_t(sizeof(keys) / sizeof(keys[0]))); cmd.addData(uint16_t(0));
        for (const auto key : keys) cmd.addData(key);
        sendCommand(cmd);
    }

    void _Livox2::handleCtrlCmdAck(const LIVOX2_CMD &cmd)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (cmd.cmd_id == LVX2_CMD_PUSH) { handlePushCmd(cmd); return; }
        if (cmd.cmd_type != LVX2_CMD_ACK || cmd.length < 27) return;
        auto found = m_pending.find(cmd.seq_num);
        if (found == m_pending.end() || found->second.command.cmd_id != cmd.cmd_id) return;
        m_pending.erase(found);
        m_lastRx = getTns();
        if (cmd.data[0])
        {
            ++m_commandErrors;
            if (cmd.cmd_id == LVX2_CMD_SET) m_configFailed = true;
            m_lastError = "LiDAR returned code " + std::to_string(cmd.data[0]) + " for command " + std::to_string(cmd.cmd_id);
            if (cmd.cmd_id == LVX2_CMD_SET) m_lastError += ", key " + std::to_string(livox2::u16(cmd.data + 1));
            return;
        }
        if (cmd.cmd_id == LVX2_CMD_GET)
            handleInfo(cmd.data + 3, cmd.length - 27, livox2::u16(cmd.data + 1));
        bool setting = false;
        for (const auto &entry : m_pending) setting |= entry.second.command.cmd_id == LVX2_CMD_SET;
        if (!setting && !m_configFailed) m_lvxState = lvxState_work;
    }

    void _Livox2::handleInfo(const uint8_t *data, size_t length, uint16_t count)
    {
        // Validate the entire TLV list before exposing any of its values.
        size_t offset = 0;
        for (uint16_t i = 0; i < count; ++i)
        {
            if (offset + 4 > length) { ++m_invalidPackets; return; }
            const size_t size = livox2::u16(data + offset + 2);
            if (size > length - offset - 4) { ++m_invalidPackets; return; }
            offset += 4 + size;
        }
        if (offset != length) { ++m_invalidPackets; return; }
        offset = 0;
        for (uint16_t i = 0; i < count; ++i)
        {
            const uint16_t key = livox2::u16(data + offset), size = livox2::u16(data + offset + 2);
            const uint8_t *value = data + offset + 4;
            const std::pair<uint16_t, const char *> byteKeys[] = {{kKeyPclDataType, "lvxPCLdataType"},
                {kKeyPatternMode, "lvxPatternMode"}, {kKeyDetectMode, "lvxDetectMode"},
                {kKeyWorkMode, "lvxWorkMode"}, {kKeyImuDataEn, "lvxIMUdataEn"}, {kKeyCurWorkState, "workState"}};
            for (const auto &field : byteKeys)
                if (key == field.first && size >= 1) m_observed[field.second] = value[0];
            if (key >= kKeyStateInfoHostIpCfg && key <= kKeyLidarImuHostIpCfg && size == 8)
            {
                uint32_t address; memcpy(&address, value, 4);
                m_observed[std::to_string(key)] = {{"ip", ipString(address)}, {"port", livox2::u16(value + 4)}};
            }
            if (key == kKeyLidarDiagStatus && size >= 2) m_observed["diagnosticStatus"] = livox2::u16(value);
            if (key == kKeyHmsCode) m_observed["hms"] = vector<uint8_t>(value, value + size);
            offset += 4 + size;
        }
    }

    void _Livox2::handlePushCmd(const LIVOX2_CMD &cmd)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (cmd.cmd_id != LVX2_CMD_PUSH || cmd.length < 28) return;
        handleInfo(cmd.data + 4, cmd.length - 28, livox2::u16(cmd.data));
        if (m_lvxState != lvxState_deviceQuery) m_lastRx = getTns();
    }
    void _Livox2::resetFrame()
    {
        m_vFramePoints.clear(); m_tPCLframe = 0; m_tLastPCL = 0; m_bPCLframe = false;
    }

    void _Livox2::handlePointCloudData(const LIVOX2_DATA &data)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (!m_pPCLout || data.data_type == kLivoxLidarImuData) return;
        const uint64_t stamp = livox2::u64(data.timestamp);
        m_lastRx = getTns();
        ++m_pointPackets;
        if (m_tLastPCL && stamp <= m_tLastPCL)
        {
            ++m_outOfOrderPackets;
            // A new clock epoch needs a fresh frame. Small reversals are
            // reordered UDP datagrams and must never enter a SLAM scan.
            if (m_tLastPCL - stamp < NSEC_SEC) return;
            resetFrame();
        }
        m_tLastPCL = stamp;
        const bool boundary = m_bPCLframe && (stamp - m_tPCLframe >= m_frameDurationNs ||
            (m_lvxCfg.m_patternMode != kLivoxLidarScanPatternNoneRepetive && data.frame_cnt != m_iPCLframe));
        if (boundary)
        {
            m_pPCLout->set(m_vFramePoints, m_tPCLframe); ++m_frames;
            m_vFramePoints.clear(); m_bPCLframe = false;
        }
        if (!m_bPCLframe)
        {
            m_tPCLframe = stamp; m_iPCLframe = data.frame_cnt; m_bPCLframe = true;
        }
        const size_t stride = livox2::pointSize(data.data_type);
        bool overflow = false;
        for (size_t i = 0; i < data.dot_num; ++i)
        {
            const uint8_t *bytes = data.data + i * stride;
            Vector3f position;
            uint8_t reflectivity;
            if (data.data_type == kLivoxLidarCartesianCoordinateHighData)
            {
                LivoxLidarCartesianHighRawPoint point; memcpy(&point, bytes, sizeof(point));
                position = Vector3f(point.x, point.y, point.z) * 0.001f;
                reflectivity = point.reflectivity;
            }
            else if (data.data_type == kLivoxLidarCartesianCoordinateLowData)
            {
                LivoxLidarCartesianLowRawPoint point; memcpy(&point, bytes, sizeof(point));
                position = Vector3f(point.x, point.y, point.z) * 0.01f;
                reflectivity = point.reflectivity;
            }
            else if (data.data_type == kLivoxLidarSphericalCoordinateData)
            {
                LivoxLidarSpherPoint point; memcpy(&point, bytes, sizeof(point));
                if (point.theta > 18000 || point.phi > 36000) continue;
                const float theta = point.theta * (float(M_PI) / 18000.0f), phi = point.phi * (float(M_PI) / 18000.0f);
                const float distance = point.depth * 0.001f;
                position = Vector3f(distance * std::sin(theta) * std::cos(phi),
                    distance * std::sin(theta) * std::sin(phi), distance * std::cos(theta));
                reflectivity = point.reflectivity;
            }
            else return;
            const float distance2 = position.squaredNorm();
            if (!position.allFinite() || distance2 <= 0 || distance2 < m_vRangeD.x() * m_vRangeD.x() ||
                distance2 > m_vRangeD.y() * m_vRangeD.y()) continue;
            if (m_vFramePoints.size() >= size_t(m_nMaxFramePoints)) { overflow = true; continue; }
            GEOMETRY_POINT point;
            point.m_vP = m_mPosef * position;
            point.m_vC = m_bReflectivity ? Vector3f::Constant(reflectivity / 255.0f) : m_vColorDefault;
            point.m_tStamp = livox2::pointTime(stamp, data.time_interval, data.dot_num, i);
            m_vFramePoints.push_back(point); ++m_points;
        }
        if (overflow) ++m_frameOverflows;
    }

    void _Livox2::clear()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        resetFrame();
        if (m_pPCLout) m_pPCLout->set({});
    }

    void _Livox2::handleIMUdata(const LIVOX2_DATA &data)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (!m_lvxCfg.m_imuDataEn || data.data_type != kLivoxLidarImuData) return;
        const uint64_t first = livox2::u64(data.timestamp);
        for (size_t i = 0; i < data.dot_num; ++i)
        {
            LivoxLidarImuRawPoint imu; memcpy(&imu, data.data + i * sizeof(imu), sizeof(imu));
            const uint64_t stamp = livox2::pointTime(first, data.time_interval, data.dot_num, i);
            const Vector3f acc = Vector3f(imu.acc_x, imu.acc_y, imu.acc_z) * 9.80665f;
            const Vector3f gyro(imu.gyro_x, imu.gyro_y, imu.gyro_z);
            if (!acc.allFinite() || !gyro.allFinite()) { ++m_invalidPackets; continue; }
            if (m_tIMU && stamp <= m_tIMU)
            {
                ++m_outOfOrderPackets;
                if (m_tIMU - stamp < NSEC_SEC) continue;
                m_imuPreview.reset();
                if (m_pIMUout) m_pIMUout->clear();
            }
            m_tIMU = stamp; m_lastIMUrx = getTns(); m_lastRx = m_lastIMUrx; ++m_imuSamples;
            if (m_pIMUout) { m_pIMUout->addAcc(acc, stamp); m_pIMUout->addGyro(gyro, stamp); }
            m_imuPreview.addAcc(acc, stamp); m_imuPreview.addGyro(gyro, stamp);
            if (m_bIMUstab)
            {
                const auto preview = m_imuPreview.snapshot();
                if (preview.orientationValid) setAngles(preview.rpy.x(), preview.rpy.y(), 0.0);
            }
        }
    }

    LVX2_CONFIG _Livox2::getConfig()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return m_lvxCfg;
    }

    void _Livox2::setConfig(const LVX2_CONFIG &cfg)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        json errors;
        const json patch = {{"lvxPCLdataType", cfg.m_pclDataType}, {"lvxPatternMode", cfg.m_patternMode},
            {"lvxDetectMode", cfg.m_detectMode}, {"lvxWorkMode", cfg.m_workMode}, {"lvxIMUdataEn", cfg.m_imuDataEn},
            {"lvxHostIP", ipString(cfg.m_hostIP)}, {"lvxHostPortState", cfg.m_hostPortState},
            {"lvxHostPortPCL", cfg.m_hostPortPCL}, {"lvxHostPortIMU", cfg.m_hostPortIMU}};
        if (!applyConfig(patch, true, errors)) LOG_E(errors.dump());
    }

    json _Livox2::configValues() const
    {
        return {{"lvxSN", m_lvxSN}, {"lvxIP", ipString(m_lvxIP)}, {"lvxHostIP", ipString(m_lvxCfg.m_hostIP)},
            {"lvxHostPortState", m_lvxCfg.m_hostPortState}, {"lvxHostPortPCL", m_lvxCfg.m_hostPortPCL},
            {"lvxHostPortIMU", m_lvxCfg.m_hostPortIMU}, {"lvxPCLdataType", m_lvxCfg.m_pclDataType},
            {"lvxPatternMode", m_lvxCfg.m_patternMode}, {"lvxFrameRate", m_lvxCfg.m_frameRate},
            {"lvxDetectMode", m_lvxCfg.m_detectMode}, {"lvxWorkModeAfterBoot", m_lvxCfg.m_workModeAfterBoot},
            {"lvxWorkMode", m_lvxCfg.m_workMode}, {"lvxIMUdataEn", m_lvxCfg.m_imuDataEn},
            {"bIMUstab", m_bIMUstab}, {"bReflectivity", m_bReflectivity},
            {"frameDurationMs", m_frameDurationNs / NSEC_MSEC}, {"tOutSec", m_timeoutNs / NSEC_SEC},
            {"nMaxFramePoints", m_nMaxFramePoints}, {"vColorDefault", {m_vColorDefault.x(), m_vColorDefault.y(), m_vColorDefault.z()}},
            {"vRangeD", {m_vRangeD.x(), m_vRangeD.y()}}};
    }

    json _Livox2::controlSchema() const
    {
        json schema = json::object();
        const auto choices = [](std::initializer_list<std::pair<int, const char *>> values) {
            json result = json::array();
            for (const auto &value : values) result.push_back({{"value", value.first}, {"label", value.second}});
            return result;
        };
        schema["lvxPCLdataType"] = {{"type", "integer"}, {"label", "Point format"}, {"choices", choices({{1, "Cartesian 32-bit (mm)"}, {2, "Cartesian 16-bit (cm)"}, {3, "Spherical"}})}};
        schema["lvxPatternMode"] = {{"type", "integer"}, {"label", "Scan pattern"}, {"choices", choices({{0, "Non-repetitive (Mid-360)"}})}};
        schema["lvxDetectMode"] = {{"type", "integer"}, {"label", "Detection"}, {"choices", choices({{0, "Normal"}, {1, "Sensitive"}})}};
        schema["lvxWorkMode"] = {{"type", "integer"}, {"label", "Work mode"}, {"choices", choices({{1, "Sampling"}, {2, "Idle"}, {9, "Ready"}})}};
        schema["lvxIMUdataEn"] = {{"type", "integer"}, {"label", "IMU output"}, {"choices", choices({{0, "Disabled"}, {1, "Enabled"}})}};
        schema["frameDurationMs"] = {{"type", "integer"}, {"label", "Cloud frame duration (ms)"}, {"min", 20}, {"max", 1000}, {"description", "Host grouping interval for the non-repetitive scan"}};
        schema["tOutSec"] = {{"type", "integer"}, {"label", "Disconnect timeout (s)"}, {"min", 1}, {"max", 120}};
        schema["nMaxFramePoints"] = {{"type", "integer"}, {"label", "Maximum points per frame"}, {"min", 96}, {"max", 1000000}};
        schema["bIMUstab"] = {{"type", "bool"}, {"label", "Stabilize cloud roll/pitch"}, {"description", "Leave disabled for SLAM; SLAM performs its own IMU fusion"}};
        schema["bReflectivity"] = {{"type", "bool"}, {"label", "Reflectivity coloring"}};
        schema["vColorDefault"] = {{"type", "array"}, {"label", "Default RGB color"}, {"length", 3}, {"min", 0}, {"max", 1}};
        schema["vRangeD"] = {{"type", "range"}, {"label", "Distance range (m)"}, {"length", 2}, {"min", 0}, {"max", 10000}};
        for (const char *key : {"lvxSN", "lvxIP", "lvxHostIP", "lvxHostPortState", "lvxHostPortPCL", "lvxHostPortIMU"})
            schema[key] = {{"type", "string"}, {"readOnly", true}, {"category", "Network"}, {"description", "Configure together with the UDP endpoints in the JSON file"}};
        for (const char *key : {"lvxFrameRate", "lvxWorkModeAfterBoot"})
            schema[key] = {{"type", "integer"}, {"readOnly", true}, {"description", "Legacy HAP setting; unsupported and unused on Mid-360"}};
        return schema;
    }

    bool _Livox2::applyConfig(const json &patch, bool live, json &errors)
    {
        errors = json::object();
        if (!patch.is_object()) { errors["config"] = "Expected an object"; return false; }
        const json before = configValues(), schema = controlSchema();
        json next = before;
        for (const auto &field : patch.items())
        {
            const string key = field.key(); const json &value = field.value();
            if (!schema.contains(key)) { errors[key] = "Unknown parameter"; continue; }
            const json &spec = schema.at(key);
            if (live && spec.value("readOnly", false) && value != before.at(key))
            { errors[key] = "Edit the JSON configuration and restart"; continue; }
            bool valid = true;
            if (key == "lvxSN") valid = value.is_string() && value.get<string>().size() <= 16;
            else if (key == "lvxIP" || key == "lvxHostIP")
            {
                uint32_t address = 0;
                valid = readIP(value, address) && (key == "lvxIP" || address != 0);
            }
            else if (key == "lvxHostPortState" || key == "lvxHostPortPCL" || key == "lvxHostPortIMU")
                valid = value.is_number_integer() && value.get<double>() >= 1 && value.get<double>() <= 65535;
            else if (spec.value("type", "") == "bool") valid = value.is_boolean();
            else if (spec.contains("choices"))
            {
                valid = false;
                if (value.is_number_integer())
                    for (const auto &choice : spec["choices"]) valid |= value == choice["value"];
            }
            else if (spec.value("type", "") == "array" || spec.value("type", "") == "range")
            {
                valid = value.is_array() && value.size() == spec.at("length").get<size_t>();
                if (valid) for (const auto &element : value)
                    valid &= element.is_number() && std::isfinite(element.get<double>()) &&
                        element.get<double>() >= spec.at("min").get<double>() && element.get<double>() <= spec.at("max").get<double>();
                if (valid && key == "vRangeD") valid = value[0].get<double>() < value[1].get<double>();
            }
            else
            {
                valid = value.is_number_integer() && value.get<double>() >= spec.value("min", 0.0) && value.get<double>() <= spec.value("max", 255.0);
            }
            if (!valid) { errors[key] = "Invalid value or outside supported range"; continue; }
            next[key] = value;
        }
        if (!errors.empty()) return false;
        m_lvxSN = next.at("lvxSN");
        readIP(next.at("lvxIP"), m_lvxIP); readIP(next.at("lvxHostIP"), m_lvxCfg.m_hostIP);
        m_lvxCfg.m_hostPortState = next.at("lvxHostPortState"); m_lvxCfg.m_hostPortPCL = next.at("lvxHostPortPCL"); m_lvxCfg.m_hostPortIMU = next.at("lvxHostPortIMU");
        m_lvxCfg.m_pclDataType = next.at("lvxPCLdataType"); m_lvxCfg.m_patternMode = next.at("lvxPatternMode");
        m_lvxCfg.m_detectMode = next.at("lvxDetectMode"); m_lvxCfg.m_workMode = next.at("lvxWorkMode");
        m_lvxCfg.m_imuDataEn = next.at("lvxIMUdataEn"); m_lvxCfg.m_frameRate = next.at("lvxFrameRate"); m_lvxCfg.m_workModeAfterBoot = next.at("lvxWorkModeAfterBoot");
        m_bIMUstab = next.at("bIMUstab"); m_bReflectivity = next.at("bReflectivity");
        m_frameDurationNs = next.at("frameDurationMs").get<uint64_t>() * NSEC_MSEC;
        m_timeoutNs = next.at("tOutSec").get<uint64_t>() * NSEC_SEC;
        m_nMaxFramePoints = next.at("nMaxFramePoints");
        for (int i = 0; i < 3; ++i) m_vColorDefault[i] = next.at("vColorDefault")[i];
        for (int i = 0; i < 2; ++i) m_vRangeD[i] = next.at("vRangeD")[i];
        if (before != next)
        {
            resetFrame();
            if (before.at("lvxIMUdataEn") != next.at("lvxIMUdataEn"))
            { m_imuPreview.reset(); m_tIMU = 0; if (m_pIMUout) m_pIMUout->clear(); }
            if (!m_bIMUstab && before.at("bIMUstab") != next.at("bIMUstab"))
            {
                // Restore the configured mounting transform after preview stabilization.
                setOrientation(m_mountOrientation, true);
            }
        }
        if (live && m_lvxState != lvxState_deviceQuery)
        {
            const std::pair<const char *, uint16_t> keys[] = {{"lvxPCLdataType", kKeyPclDataType},
                {"lvxPatternMode", kKeyPatternMode}, {"lvxDetectMode", kKeyDetectMode},
                {"lvxWorkMode", kKeyWorkMode}, {"lvxIMUdataEn", kKeyImuDataEn}};
            if (m_configFailed && !patch.empty())
            {
                // The desired values remain visible after a device rejection.
                // An explicit retry (including the same values) must reapply
                // ALL desired settings before reporting configuration success.
                // This also prevents an unrelated edit from hiding a failure.
                for (auto it = m_pending.begin(); it != m_pending.end();)
                    if (it->second.command.cmd_id == LVX2_CMD_SET) it = m_pending.erase(it);
                    else ++it;
                m_configFailed = false; m_lastError.clear(); m_lvxState = lvxState_init;
                configureDevice();
            }
            else
            {
                bool changes = false;
                for (const auto &key : keys) changes |= before.at(key.first) != next.at(key.first);
                if (changes) m_lastError.clear();
                for (const auto &key : keys)
                    if (before.at(key.first) != next.at(key.first)) setLvxByte(key.second, next.at(key.first).get<uint8_t>());
            }
        }
        return true;
    }

    json _Livox2::imuValues() const
    {
        const auto sample = m_imuPreview.snapshot();
        const uint64_t now = getTns();
        const bool open = m_lvxState != lvxState_deviceQuery;
        const bool available = open && m_lvxCfg.m_imuDataEn && sample.tGyro && m_lastIMUrx && now - m_lastIMUrx < NSEC_SEC / 2;
        return {{"enabled", bool(m_lvxCfg.m_imuDataEn)}, {"deviceOpen", open}, {"available", available},
            {"tGyro", std::to_string(sample.tGyro)}, {"tAcc", std::to_string(sample.tAcc)}, {"tFusion", std::to_string(sample.tFusion)},
            {"gyro", {sample.gyro.x(), sample.gyro.y(), sample.gyro.z()}}, {"acc", {sample.acc.x(), sample.acc.y(), sample.acc.z()}},
            {"quaternion", {sample.quaternion.w(), sample.quaternion.x(), sample.quaternion.y(), sample.quaternion.z()}},
            {"rpy", {sample.rpy.x(), sample.rpy.y(), sample.rpy.z()}}, {"fusion", true}, {"orientationValid", available && sample.orientationValid}};
    }

    void _Livox2::console(void *console)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        _TransformBase::console(console);
        if (console) static_cast<_Console *>(console)->addMsg("Livox state=" + std::to_string(m_lvxState) + " SN=" + m_detectedSN +
            " frames=" + std::to_string(m_frames) + " IMU=" + std::to_string(m_imuSamples) + " " + m_lastError);
    }

    void _Livox2::console(const json &request, void *pJSONbase)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto *endpoint = static_cast<_JSONbase *>(pJSONbase);
        if (!endpoint || !request.is_object() || !request.contains("cmd") || !request.at("cmd").is_string()) return;
        const string command = request.at("cmd");
        if (command != "getConfig" && command != "setConfig" && command != "saveConfig" && command != "getIMU") return;
        json reply = {{"cmd", command}, {"module", getName()}, {"bSuccess", true}};
        if (request.contains("requestId")) reply["requestId"] = request.at("requestId");
        try
        {
            if (command == "getIMU") reply.update(imuValues());
            else
            {
                if (command == "setConfig")
                {
                    json errors;
                    reply["bSuccess"] = applyConfig(request.value("config", json()), true, errors);
                    reply["errors"] = errors;
                }
                else if (command == "saveConfig") reply["bSuccess"] = saveConfig(true);
                size_t setting = 0;
                for (const auto &entry : m_pending) setting += entry.second.command.cmd_id == LVX2_CMD_SET;
                reply["config"] = configValues(); reply["schema"] = controlSchema();
                reply["deviceOpen"] = m_lvxState != lvxState_deviceQuery;
                reply["status"] = {{"state", m_lvxState == lvxState_deviceQuery ? "discovering" : m_lvxState == lvxState_init ? "configuring" : "connected"},
                    {"serialNumber", m_detectedSN}, {"deviceType", m_lvxDevType}, {"commandPort", m_lvxCmdPort},
                    {"pointPackets", m_pointPackets}, {"points", m_points}, {"frames", m_frames}, {"imuSamples", m_imuSamples},
                    {"invalidPackets", m_invalidPackets}, {"outOfOrderPackets", m_outOfOrderPackets}, {"frameOverflows", m_frameOverflows},
                    {"commandErrors", m_commandErrors}, {"commandRetries", m_commandRetries},
                    {"pendingCommands", m_pending.size()}, {"configApplied", m_lvxState != lvxState_deviceQuery && !setting && !m_configFailed},
                    {"lastError", m_lastError}, {"observed", m_observed}};
                if (!m_lastError.empty()) reply["error"] = m_lastError;
            }
        }
        catch (const std::exception &e) { reply["bSuccess"] = false; reply["error"] = e.what(); }
        endpoint->sendJson(reply);
    }
}
