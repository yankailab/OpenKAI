/* SDK-free Livox legacy Ethernet driver for Avia and standalone first-generation LiDARs. */
#include "_Livox1.h"
#include "../../../../Protocol/_JSONbase.h"
#include <poll.h>

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

    _Livox1::_Livox1() = default;
    _Livox1::~_Livox1() { stop(); }

    void _Livox1::stop()
    {
        // The worker owns the descriptors until it exits. Never join under its state lock.
        if (m_pT) { m_pT->stop(); m_pT->join(); }
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        closeSockets();
        m_started = false;
        m_state = State::Discovering;
        m_pending.clear();
        resetFrame();
        if (m_bIMUpreview) m_imuPreview.reset();
        m_tIMU = m_lastIMUrx = 0;
    }

    bool _Livox1::loadConfig()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        IF_Le_F(m_started, "Stop Livox before loading its configuration");
        IF_F(!_ModuleBase::loadConfig());
        if (!m_pJ->at("thread").contains("FPS")) m_pT->setTargetFPS(0);
        json patch = json::object(), errors;
        const json values = configValues();
        for (const auto &entry : values.items())
            if (m_pJ->contains(entry.key())) patch[entry.key()] = (*m_pJ)[entry.key()];
        if (!applyConfig(patch, false, errors)) { LOG_E(errors.dump()); return false; }
        IF_Le_F(!m_lvxIP, "Set lvxIP to the LiDAR IPv4 address");
        m_vFramePoints.reserve(m_nMaxFramePoints);
        return true;
    }

    bool _Livox1::saveConfig(bool bExport)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        IF_F(!_ModuleBase::saveConfig(false));
        m_pJ->update(configValues());
        return !bExport || m_pJcfg->saveToFile();
    }

    bool _Livox1::link(InstanceMgr *pM)
    {
        IF_F(!_ModuleBase::link(pM));

        string tfName;
        jKv(*m_pJ, "_TransformRigid", tfName);
        m_pTf = dynamic_cast<_TransformRigid *>(static_cast<_ModuleBase *>(pM->findModule(tfName)));
        IF_Le_F(!tfName.empty() && !m_pTf, "_TransformRigid not found: " + tfName);
        m_mountOrientation = m_pTf ? m_pTf->getRotation() : Quaterniond::Identity();

        string name;
        jKv(*m_pJ, "PCLframeOut", name);
        m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(name)));
        IF_Le_F(!m_pPCLout, "PCLframeOut not found: " + name);
        name.clear();
        jKv(*m_pJ, "IMUstreamOut", name);
        m_pIMUout = dynamic_cast<IMUstream *>(static_cast<DataObjBase *>(pM->findDataObject(name)));
        IF_Le_F(!name.empty() && !m_pIMUout, "IMUstreamOut not found: " + name);
        return true;
    }

    bool _Livox1::check() { return m_pPCLout && m_pT && _ModuleBase::check(); }

    bool _Livox1::openSockets(string &error)
    {
        error.clear();
        if (!m_lvxIP) { error = "Set lvxIP to the LiDAR IPv4 address"; return false; }
        uint32_t hostIP = m_lvxCfg.m_hostIP;
        if (!hostIP)
        {
            // Use the route to the configured LiDAR to choose the handshake IP.
            const int probe = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
            if (probe < 0) { error = "Livox route socket: " + string(strerror(errno)); return false; }
            sockaddr_in address{};
            address.sin_family = AF_INET; address.sin_addr.s_addr = m_lvxIP; address.sin_port = htons(m_lvxCmdPort);
            if (connect(probe, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
            { error = "Livox route: " + string(strerror(errno)); close(probe); return false; }
            socklen_t size = sizeof(address);
            if (getsockname(probe, reinterpret_cast<sockaddr *>(&address), &size) < 0 || !address.sin_addr.s_addr)
            { error = "Cannot determine Livox host IP"; close(probe); return false; }
            hostIP = address.sin_addr.s_addr;
            close(probe);
        }
        std::array<int, SocketCount> opened{{-1, -1, -1, -1}};
        const auto fail = [&]() {
            for (const int fd : opened) if (fd >= 0) close(fd);
            return false;
        };
        const uint16_t ports[] = {m_broadcastPort, m_hostPortCmd, m_lvxCfg.m_hostPortPCL, m_lvxCfg.m_hostPortIMU};
        // Reserve the unicast ports before the shared discovery listener.
        for (size_t order = 1; order <= opened.size(); ++order)
        {
            const size_t i = order % opened.size();
            const int fd = opened[i] = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
            if (fd < 0) { error = "Livox UDP socket: " + string(strerror(errno)); return fail(); }
            if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &m_receiveBufferBytes, sizeof(m_receiveBufferBytes)) < 0)
            { error = "Livox UDP receive buffer: " + string(strerror(errno)); return fail(); }
            sockaddr_in address{};
            address.sin_family = AF_INET;
            // Legacy devices announce themselves by broadcast on UDP 55000.
            address.sin_addr.s_addr = i == Broadcast ? INADDR_ANY : hostIP;
            address.sin_port = htons(ports[i]);
            if (i == Broadcast)
            {
                // Broadcast discovery can be shared by multiple Livox instances;
                // each instance filters by its configured LiDAR address and serial.
                const int reuse = 1;
                if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
                { error = "Livox discovery socket: " + string(strerror(errno)); return fail(); }
            }
            // Unicast channels bind exclusively to expose conflicting instances.
            if (bind(fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
            {
                error = "Livox bind " + ipString(address.sin_addr.s_addr) + ":" + std::to_string(ports[i]) + ": " + strerror(errno);
                return fail();
            }
        }
        closeSockets();
        m_sockets = opened;
        m_hostIP = hostIP;
        return true;
    }

    void _Livox1::closeSockets()
    {
        for (int &fd : m_sockets) { if (fd >= 0) close(fd); fd = -1; }
        m_hostIP = 0;
    }

    bool _Livox1::start()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (m_started) return true;
        if (!check()) return false;
        m_state = State::Discovering;
        m_lastRx = m_lastHeartbeat = m_lastHandshake = m_lastIMUrx = 0;
        m_lvxDevType = livox1::Unknown; m_firmwareKnown = false; m_workState = 0;
        resetClocks();
        m_detectedSN.clear(); m_observed = json::object();
        m_pending.clear(); m_configFailed = false;
        resetFrame();
        if (m_bIMUpreview) m_imuPreview.reset();
        m_tIMU = 0;
        if (m_pIMUout) m_pIMUout->clear();
        m_lastError.clear();
        m_started = true;
        if (m_pT->startThread(getUpdate, this)) return true;
        m_started = false;
        closeSockets();
        m_lastError = "Could not start Livox receive thread";
        return false;
    }

    bool _Livox1::sendDatagram(const vector<uint8_t> &bytes)
    {
        if (m_sockets[Command] < 0 || bytes.empty()) return false;
        sockaddr_in remote{};
        remote.sin_family = AF_INET; remote.sin_addr.s_addr = m_lvxIP; remote.sin_port = htons(m_lvxCmdPort);
        ssize_t sent;
        do { sent = sendto(m_sockets[Command], bytes.data(), bytes.size(), 0,
            reinterpret_cast<sockaddr *>(&remote), sizeof(remote)); } while (sent < 0 && errno == EINTR);
        if (sent == ssize_t(bytes.size())) return true;
        ++m_commandErrors;
        m_lastError = "Livox1 UDP send: " + string(sent < 0 ? strerror(errno) : "incomplete datagram");
        return false;
    }

    void _Livox1::update()
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();
            if (!m_pT->bRun()) break;
            pollfd sockets[SocketCount]{};
            {
                std::unique_lock<std::recursive_mutex> lock(m_mutex);
                if (m_sockets[Command] < 0)
                {
                    const string previousError = m_lastError;
                    if (!openSockets(m_lastError))
                    {
                        if (m_lastError != previousError) LOG_E(m_lastError);
                        // Retry without holding state locked; stop() wakes this wait.
                        lock.unlock();
                        m_pT->sleepT(NSEC_SEC);
                        continue;
                    }
                }
                service(getTns());
                for (size_t i = 0; i < m_sockets.size(); ++i) sockets[i] = {m_sockets[i], POLLIN, 0};
            }
            const int ready = poll(sockets, SocketCount, 20);
            if (!m_pT->bRun()) break;
            if (ready < 0)
            {
                if (errno != EINTR)
                {
                    std::lock_guard<std::recursive_mutex> lock(m_mutex);
                    m_lastError = "Livox1 UDP poll: " + string(strerror(errno));
                }
                continue;
            }
            // A timeout already paced the loop; receive() skips sleep when data arrives.
            if (ready == 0) m_pT->skipSleep();
            for (size_t i = 0; i < m_sockets.size(); ++i)
            {
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
                if (sockets[i].revents & (POLLIN | POLLERR)) receive(i);
                if (sockets[i].revents & (POLLHUP | POLLNVAL)) m_lastError = "Livox1 UDP socket closed unexpectedly";
            }
        }
    }

    void _Livox1::receive(size_t channel)
    {
        for (unsigned batch = 0; batch < 256; ++batch)
        {
            uint8_t bytes[livox1::MaxPacketSize];
            sockaddr_in source{};
            socklen_t size = sizeof(source);
            const ssize_t length = recvfrom(m_sockets[channel], bytes, sizeof(bytes), MSG_TRUNC,
                reinterpret_cast<sockaddr *>(&source), &size);
            if (length < 0)
            {
                if (errno == EINTR) continue;
                if (errno != EAGAIN && errno != EWOULDBLOCK) m_lastError = "Livox1 UDP receive: " + string(strerror(errno));
                break;
            }
            m_pT->skipSleep();
            if (source.sin_addr.s_addr != m_lvxIP) continue;
            if (length > ssize_t(sizeof(bytes))) { ++m_invalidPackets; continue; }
            if (channel == Broadcast || channel == Command)
            {
                if (ntohs(source.sin_port) != m_lvxCmdPort || !recvLivoxCmd(bytes, size_t(length))) continue;
                if (bytes[9] == 0 && bytes[10] == livox1::Broadcast) handleBroadcast(bytes, size_t(length));
                else if (channel == Command) handleCtrlCmdAck(bytes, size_t(length));
            }
            else
            {
                if (m_state == State::Discovering || m_state == State::Handshaking) continue;
                if (!recvLivoxData(bytes, size_t(length))) continue;
                if ((bytes[9] == 6) != (channel == Imu) || !livox1::supportsData(m_lvxDevType, bytes[9]))
                { ++m_invalidPackets; continue; }
                if (channel == PointCloud) handlePointCloudData(bytes, size_t(length));
                else handleIMUdata(bytes, size_t(length));
            }
        }
    }

    bool _Livox1::recvLivoxCmd(const uint8_t *bytes, size_t length)
    {
        if (livox1::validCommand(bytes, length)) return true;
        ++m_invalidPackets; return false;
    }

    bool _Livox1::recvLivoxData(const uint8_t *bytes, size_t length)
    {
        if (livox1::validData(bytes, length)) return true;
        ++m_invalidPackets; return false;
    }

    void _Livox1::reconnect(const string &reason)
    {
        m_state = State::Discovering;
        m_pending.clear(); m_firmwareKnown = false; m_configFailed = false; m_samplingSent = false;
        m_lastRx = m_lastHeartbeat = m_lastHandshake = 0; m_workState = 0;
        m_lastError = reason;
        if (m_pPCLout) m_pPCLout->set({}, m_tLastPCL ? m_tLastPCL : 1);
        resetClocks();
        if (m_pIMUout) m_pIMUout->clear();
    }

    void _Livox1::service(uint64_t now)
    {
        if (m_state != State::Discovering && m_lastRx && now - m_lastRx > m_timeoutNs)
        { reconnect("LiDAR timed out; waiting for its broadcast"); return; }
        if (m_state == State::Discovering) return;
        for (auto it = m_pending.begin(); it != m_pending.end();)
        {
            auto &pending = it->second;
            const uint64_t interval = pending.switching ? m_timeoutNs : 500 * NSEC_MSEC;
            if (now - pending.sent < interval) { ++it; continue; }
            const uint8_t set = pending.bytes[9], id = pending.bytes[10];
            if (pending.attempts >= 3 || pending.switching)
            {
                ++m_commandErrors;
                const string error = "Livox1 command " + std::to_string(set) + ":" + std::to_string(id) + " timed out";
                if (set == 0 && (id == livox1::Handshake || id == livox1::DeviceInfo || id == livox1::Heartbeat))
                { reconnect(error + "; waiting for its broadcast"); return; }
                if (pending.setting) m_configFailed = true;
                m_lastError = error;
                it = m_pending.erase(it); continue;
            }
            ++pending.attempts; ++m_commandRetries; pending.sent = now;
            if (set == 0 && id == livox1::Handshake)
            {
                // Released SDKs use ACK for the handshake request, whereas the
                // written protocol specifies CMD. Retry the documented form.
                vector<uint8_t> payload(pending.bytes.begin() + 11, pending.bytes.end() - 4);
                pending.bytes = livox1::makeCommand(set, id, livox1::Request, it->first, payload);
            }
            sendDatagram(pending.bytes); ++it;
        }
        if (m_state != State::Handshaking && (!m_lastHeartbeat || now - m_lastHeartbeat >= NSEC_SEC))
        {
            bool waiting = false;
            for (const auto &entry : m_pending)
                waiting |= entry.second.bytes[9] == 0 && entry.second.bytes[10] == livox1::Heartbeat;
            if (!waiting) sendCommand(0, livox1::Heartbeat);
            m_lastHeartbeat = now;
        }
    }

    void _Livox1::sendCommand(uint8_t set, uint8_t id, const vector<uint8_t> &payload, bool setting, const string &key)
    {
        if (m_sockets[Command] < 0) return;
        // Sequence rollover must not replace an outstanding request.
        do { ++m_sequence; } while (m_pending.count(m_sequence));
        const uint8_t type = set == 0 && id == livox1::Handshake ? livox1::Ack : livox1::Request;
        PendingCommand pending;
        pending.bytes = livox1::makeCommand(set, id, type, m_sequence, payload);
        pending.sent = getTns(); pending.setting = setting; pending.key = key;
        m_pending.emplace(m_sequence, pending);
        sendDatagram(pending.bytes);
    }

    void _Livox1::sendHandshake()
    {
        vector<uint8_t> payload(livox1::hasImu(m_lvxDevType) ? 10 : 8);
        memcpy(payload.data(), &m_hostIP, 4);
        livox1::put16(payload.data() + 4, m_lvxCfg.m_hostPortPCL);
        livox1::put16(payload.data() + 6, m_hostPortCmd);
        if (payload.size() == 10) livox1::put16(payload.data() + 8, m_lvxCfg.m_hostPortIMU);
        m_state = State::Handshaking;
        m_lastRx = m_lastHandshake = getTns();
        sendCommand(0, livox1::Handshake, payload);
    }

    void _Livox1::handleBroadcast(const uint8_t *bytes, size_t length)
    {
        if (!livox1::validCommand(bytes, length) || bytes[4] != livox1::Message || bytes[9] != 0 ||
            bytes[10] != livox1::Broadcast || length != 34) { ++m_invalidPackets; return; }
        const uint8_t *payload = bytes + 11;
        const string serial(reinterpret_cast<const char *>(payload), strnlen(reinterpret_cast<const char *>(payload), 16));
        if (!m_lvxSN.empty() && serial != m_lvxSN) return;
        if (!livox1::supportedDevice(payload[16]))
        { m_lastError = "Unsupported legacy device (standalone Mid-40, Mid-70, Tele-15, Horizon or Avia required)"; return; }
        if (m_state == State::Handshaking) return;
        if (m_state != State::Discovering) reconnect("LiDAR restarted; reconnecting");
        m_detectedSN = serial; m_lvxDevType = payload[16]; m_observed = json::object();
        m_lastError.clear();
        sendHandshake();
    }

    bool _Livox1::supportsSetting(const string &key) const
    {
        if (m_lvxDevType == livox1::Unknown) return true;
        if (key == "lvxScanPattern" && m_lvxDevType != livox1::Avia) return false;
        if (key == "lvxIMUdataEn" && !livox1::hasImu(m_lvxDevType)) return false;
        if (key == "lvxFan" && !livox1::hasFan(m_lvxDevType)) return false;
        if (key == "lvxReturnMode" && m_lvxDevType == livox1::Mid40) return false;
        if (key == "lvxHighSensitivity" && m_lvxDevType != livox1::Tele15 && m_lvxDevType != livox1::Avia) return false;
        if (key == "lvxFan" || key == "lvxReturnMode" || key == "lvxIMUdataEn" || key == "lvxScanPattern" || key == "lvxHighSensitivity")
        {
            const uint8_t minMajor = m_lvxDevType == livox1::Mid40 ? 3 : m_lvxDevType == livox1::Horizon ? 6 :
                m_lvxDevType == livox1::Tele15 ? 7 : m_lvxDevType == livox1::Mid70 ? 10 : 11;
            const uint8_t minMinor = m_lvxDevType == livox1::Mid40 ? 7 : m_lvxDevType == livox1::Horizon ? 4 :
                m_lvxDevType == livox1::Avia ? 6 : key == "lvxHighSensitivity" ? 9 : 3;
            return !m_firmwareKnown || m_firmware[0] > minMajor || (m_firmware[0] == minMajor && m_firmware[1] >= minMinor);
        }
        return true;
    }

    void _Livox1::querySetting(const string &key)
    {
        if (!supportsSetting(key)) return;
        if (key == "lvxReturnMode") sendCommand(1, livox1::GetReturn, {}, true, key);
        else if (key == "lvxIMUdataEn") sendCommand(1, livox1::GetImu, {}, true, key);
        else if (key == "lvxFan") sendCommand(1, livox1::GetFan, {}, true, key);
        else if (key == "lvxScanPattern") sendCommand(0, livox1::ReadParameter, {1, 2, 0}, true, key);
        else if (key == "lvxHighSensitivity") sendCommand(0, livox1::ReadParameter, {1, 1, 0}, true, key);
    }

    void _Livox1::setLvxValue(const string &key)
    {
        if (!supportsSetting(key)) return;
        if (key == "lvxCoordinate") sendCommand(0, livox1::Coordinate, {m_lvxCfg.m_coordinate}, true, key);
        else if (key == "lvxSampling") sendCommand(0, livox1::Sampling, {uint8_t(m_lvxCfg.m_sampling)}, true, key);
        else if (key == "lvxReturnMode") sendCommand(1, livox1::SetReturn, {m_lvxCfg.m_returnMode}, true, key);
        else if (key == "lvxIMUdataEn") sendCommand(1, livox1::SetImu, {m_lvxCfg.m_imuDataEn}, true, key);
        else if (key == "lvxFan") sendCommand(1, livox1::SetFan, {uint8_t(m_lvxCfg.m_fan)}, true, key);
        else if (key == "lvxWorkMode") sendCommand(1, livox1::WorkMode, {m_lvxCfg.m_workMode}, true, key);
        else if (key == "lvxScanPattern") sendCommand(0, livox1::WriteParameter, {2, 0, 1, 0, m_lvxCfg.m_scanPattern}, true, key);
        else if (key == "lvxHighSensitivity") sendCommand(0, livox1::WriteParameter, {1, 0, 1, 0, uint8_t(m_lvxCfg.m_highSensitivity)}, true, key);
    }

    void _Livox1::configureDevice()
    {
        m_state = State::Configuring; m_configFailed = false; m_samplingSent = false;
        if ((m_lvxCfg.m_returnMode == 3 && m_lvxDevType != livox1::Avia) ||
            (m_lvxCfg.m_scanPattern && m_lvxDevType != livox1::Avia) ||
            (m_lvxCfg.m_returnMode && !supportsSetting("lvxReturnMode")) ||
            (m_lvxCfg.m_scanPattern && !supportsSetting("lvxScanPattern")))
        { m_configFailed = true; m_lastError = "Requested return mode or scan pattern is unsupported by this model"; return; }
        setLvxValue("lvxCoordinate");
        if (m_workState != m_lvxCfg.m_workMode) setLvxValue("lvxWorkMode");
        // These settings persist in the device's flash. Read before writing so
        // restarts and browser reconnects do not cause unnecessary flash writes.
        for (const char *key : {"lvxReturnMode", "lvxIMUdataEn", "lvxFan", "lvxScanPattern", "lvxHighSensitivity"}) querySetting(key);
        finishConfiguration();
    }

    void _Livox1::finishConfiguration()
    {
        if (m_state != State::Configuring || m_configFailed) return;
        for (const auto &entry : m_pending) if (entry.second.setting) return;
        if (!m_samplingSent)
        {
            m_samplingSent = true;
            // Sampling is sent after work-mode completion and all format ACKs.
            // In power-saving/standby retain the desired sampling preference.
            if (m_lvxCfg.m_workMode == 1 || !m_lvxCfg.m_sampling)
            { setLvxValue("lvxSampling"); return; }
        }
        m_state = State::Connected; m_lastError.clear();
    }

    void _Livox1::handleCtrlCmdAck(const uint8_t *bytes, size_t length)
    {
        if (!livox1::validCommand(bytes, length)) { ++m_invalidPackets; return; }
        const uint8_t set = bytes[9], id = bytes[10];
        const uint8_t *payload = bytes + 11;
        const size_t payloadSize = length - livox1::CommandOverhead;
        if (bytes[4] == livox1::Message && set == 0 && id == livox1::Abnormal)
        {
            if (payloadSize != 4) { ++m_invalidPackets; return; }
            m_observed["statusCode"] = livox1::u32(payload); return;
        }
        if (bytes[4] != livox1::Ack || !payloadSize) return;
        auto found = m_pending.find(livox1::u16(bytes + 5));
        if (found == m_pending.end() || found->second.bytes[9] != set || found->second.bytes[10] != id) return;
        size_t expected = 1;
        if (set == 0 && id == livox1::DeviceInfo) expected = 5;
        else if (set == 0 && id == livox1::Heartbeat) expected = 7;
        else if (set == 0 && id == livox1::WriteParameter) expected = 4;
        else if (set == 0 && id == livox1::ReadParameter) expected = payload[0] ? 4 : 9;
        else if (set == 1 && (id == livox1::GetFan || id == livox1::GetReturn || id == livox1::GetImu)) expected = 2;
        if (payloadSize != expected) { ++m_invalidPackets; return; }
        if (set == 0 && id == livox1::Heartbeat && !payload[0] && payload[1] > 4)
        { ++m_invalidPackets; return; }
        if (set == 0 && id == livox1::ReadParameter && !payload[0] &&
            (livox1::u16(payload + 4) != (found->second.key == "lvxHighSensitivity" ? 1 : 2) ||
             livox1::u16(payload + 6) != 1 || payload[8] > 1))
        { ++m_invalidPackets; return; }
        m_lastRx = getTns();
        if (set == 1 && id == livox1::WorkMode && payload[0] == 2)
        { found->second.switching = true; found->second.sent = m_lastRx; return; }
        const PendingCommand pending = found->second;
        m_pending.erase(found);
        if (payload[0] || (set == 0 && (id == livox1::WriteParameter || id == livox1::ReadParameter) && payload[3]))
        {
            ++m_commandErrors;
            if (pending.setting) m_configFailed = true;
            m_lastError = "LiDAR rejected command " + std::to_string(set) + ":" + std::to_string(id) +
                " (" + pending.key + "), code " + std::to_string(payload[0]);
            if (set == 0 && (id == livox1::Handshake || id == livox1::DeviceInfo || id == livox1::Heartbeat)) reconnect(m_lastError);
            return;
        }
        if (set == 0 && id == livox1::Handshake)
        {
            m_state = State::Initializing;
            sendCommand(0, livox1::DeviceInfo); sendCommand(0, livox1::Heartbeat);
            m_lastHeartbeat = getTns(); return;
        }
        if (set == 0 && id == livox1::DeviceInfo)
        {
            memcpy(m_firmware.data(), payload + 1, 4); m_firmwareKnown = true;
            m_observed["firmware"] = vector<uint8_t>(m_firmware.begin(), m_firmware.end());
        }
        else if (set == 0 && id == livox1::Heartbeat)
        {
            m_workState = payload[1]; m_observed["workState"] = m_workState;
            m_observed["feature"] = payload[2]; m_observed["statusCode"] = livox1::u32(payload + 3);
            for (auto it = m_pending.begin(); it != m_pending.end();)
            {
                if (it->second.switching && m_workState == m_lvxCfg.m_workMode)
                { m_observed["lvxWorkMode"] = m_workState; it = m_pending.erase(it); }
                else ++it;
            }
        }
        else if (!pending.key.empty())
        {
            const bool query = (set == 1 && (id == livox1::GetFan || id == livox1::GetReturn || id == livox1::GetImu)) ||
                (set == 0 && id == livox1::ReadParameter);
            const json desired = configValues().at(pending.key);
            if (query)
            {
                const uint8_t value = set == 0 ? payload[8] : payload[1];
                const uint8_t maxValue = pending.key == "lvxReturnMode" ? (m_lvxDevType == livox1::Avia ? 3 : 2) : 1;
                if (value > maxValue) { ++m_invalidPackets; m_configFailed = true; m_lastError = "Invalid setting readback from LiDAR"; return; }
                m_observed[pending.key] = desired.is_boolean() ? json(bool(value)) : json(value);
                if (m_observed.at(pending.key) != desired) setLvxValue(pending.key);
            }
            else m_observed[pending.key] = desired;
        }
        if (m_state == State::Initializing && m_firmwareKnown && m_workState > 0 && m_workState < 4)
            configureDevice();
        else finishConfiguration();
    }

    void _Livox1::resetFrame()
    {
        m_vFramePoints.clear(); m_tPCLframe = m_tLastPCL = 0; m_bPCLframe = false;
    }

    void _Livox1::resetClocks()
    {
        resetFrame(); m_timeType = 255; m_ppsAnchorHost = 0; m_ppsAnchorOffset = 0;
        m_tIMU = m_lastIMUrx = 0;
        if (m_bIMUpreview) m_imuPreview.reset();
    }

    uint64_t _Livox1::dataTimestamp(const uint8_t *bytes)
    {
        const uint8_t type = bytes[8];
        if (m_timeType != type)
        {
            resetClocks(); m_timeType = type;
            if (m_pIMUout) m_pIMUout->clear();
        }
        uint64_t stamp = 0;
        if (!livox1::timestamp(bytes + 10, type, stamp)) return 0;
        if (type != 4) return stamp;
        const uint64_t now = getTns();
        const int64_t offset = livox1::i64(bytes + 10);
        if (!m_ppsAnchorHost) { m_ppsAnchorHost = now; m_ppsAnchorOffset = offset; }
        // PPS carries no date or seconds counter and can be negative for IMU.
        // Reconstruct elapsed whole seconds against the monotonic receive clock,
        // using one anchor for BOTH streams; this also tolerates reordered packets
        // across a PPS edge and idle intervals longer than one second.
        // The offset is an untrusted signed 64-bit wire value. Subtract and
        // multiply in a wider floating representation to avoid signed overflow
        // even for malformed extreme offsets. Reject results outside our clock.
        const long double elapsed = static_cast<long double>(now - m_ppsAnchorHost);
        const long double seconds = std::round((elapsed - (static_cast<long double>(offset) - m_ppsAnchorOffset)) / NSEC_SEC);
        const long double mapped = NSEC_SEC + seconds * NSEC_SEC + offset;
        return mapped > 0 && mapped < static_cast<long double>(UINT64_MAX - 1000000ULL) ? uint64_t(mapped) : 0;
    }

    void _Livox1::handlePointCloudData(const uint8_t *bytes, size_t length)
    {
        if (!m_pPCLout || !livox1::validData(bytes, length) || bytes[9] == 6) return;
        const uint64_t stamp = dataTimestamp(bytes);
        if (!stamp) { ++m_invalidPackets; return; }
        m_lastRx = getTns(); ++m_pointPackets;
        m_observed["timestampType"] = bytes[8]; m_observed["pointDataType"] = bytes[9];
        m_observed["statusCode"] = livox1::u32(bytes + 4);
        if (m_tLastPCL && stamp <= m_tLastPCL)
        {
            ++m_outOfOrderPackets;
            if (m_tLastPCL - stamp < NSEC_SEC) return;
            resetFrame();
        }
        m_tLastPCL = stamp;
        if (m_bPCLframe && stamp - m_tPCLframe >= m_frameDurationNs)
        {
            m_pPCLout->set(m_vFramePoints, m_tPCLframe); ++m_frames;
            m_vFramePoints.clear(); m_bPCLframe = false;
        }
        if (!m_bPCLframe) { m_tPCLframe = stamp; m_bPCLframe = true; }
        const uint8_t type = bytes[9];
        const size_t stride = livox1::pointSize(type), echoes = livox1::returns(type);
        const Isometry3f pose = m_pTf ? m_pTf->getPoseCombinedf() : Isometry3f::Identity();
        bool overflow = false;
        for (size_t i = 0; i < livox1::pointCount(type); ++i)
        {
            for (size_t echo = 0; echo < echoes; ++echo)
            {
                livox1::Point raw;
                if (!livox1::decodePoint(bytes + 18 + i * stride, type, echo, raw)) continue;
                const Vector3f position(raw.x, raw.y, raw.z);
                const float distance2 = position.squaredNorm();
                if (!position.allFinite() || distance2 <= 0 || distance2 < m_vRangeD.x() * m_vRangeD.x() ||
                    distance2 > m_vRangeD.y() * m_vRangeD.y()) continue;
                if (m_vFramePoints.size() >= size_t(m_nMaxFramePoints)) { overflow = true; continue; }
                GEOMETRY_POINT point;
                point.m_vP = pose * position;
                point.m_vC = m_bReflectivity ? Vector3f::Constant(raw.reflectivity / 255.0f) : m_vColorDefault;
                point.m_tStamp = livox1::pointTime(stamp, m_lvxDevType, type, i * echoes + echo);
                m_vFramePoints.push_back(point); ++m_points;
            }
        }
        if (overflow) ++m_frameOverflows;
    }

    void _Livox1::handleIMUdata(const uint8_t *bytes, size_t length)
    {
        if (!m_lvxCfg.m_imuDataEn || !livox1::validData(bytes, length) || bytes[9] != 6) return;
        const uint8_t *payload = bytes + 18;
        const Vector3f gyro(livox1::f32(payload), livox1::f32(payload + 4), livox1::f32(payload + 8));
        const Vector3f acc = Vector3f(livox1::f32(payload + 12), livox1::f32(payload + 16), livox1::f32(payload + 20)) * 9.80665f;
        if (!acc.allFinite() || !gyro.allFinite()) { ++m_invalidPackets; return; }
        const uint64_t stamp = dataTimestamp(bytes);
        if (!stamp) { ++m_invalidPackets; return; }
        if (m_tIMU && stamp <= m_tIMU)
        {
            ++m_outOfOrderPackets;
            if (m_tIMU - stamp < NSEC_SEC) return;
            if (m_bIMUpreview) m_imuPreview.reset();
            if (m_pIMUout) m_pIMUout->clear();
        }
        m_tIMU = stamp; m_lastIMUrx = getTns(); m_lastRx = m_lastIMUrx; ++m_imuSamples;
        if (m_pIMUout) { m_pIMUout->addAcc(acc, stamp); m_pIMUout->addGyro(gyro, stamp); }
        if (m_bIMUpreview)
        {
            m_imuPreview.addAcc(acc, stamp); m_imuPreview.addGyro(gyro, stamp);
            if (m_bIMUstab)
            {
                const auto preview = m_imuPreview.snapshot();
                if (preview.orientationValid && m_pTf) m_pTf->setAngles(preview.rpy.x(), preview.rpy.y(), 0.0);
            }
        }
    }

    void _Livox1::clear()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        resetFrame(); if (m_pPCLout) m_pPCLout->set({});
    }

    LVX1_CONFIG _Livox1::getConfig()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex); return m_lvxCfg;
    }

    void _Livox1::setConfig(const LVX1_CONFIG &config)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        const json patch = {{"lvxHostIP", ipString(config.m_hostIP)}, {"lvxHostPortPCL", config.m_hostPortPCL},
            {"lvxHostPortIMU", config.m_hostPortIMU}, {"lvxCoordinate", config.m_coordinate},
            {"lvxReturnMode", config.m_returnMode}, {"lvxScanPattern", config.m_scanPattern},
            {"lvxWorkMode", config.m_workMode}, {"lvxIMUdataEn", config.m_imuDataEn},
            {"lvxSampling", config.m_sampling}, {"lvxFan", config.m_fan}, {"lvxHighSensitivity", config.m_highSensitivity}};
        json errors;
        if (!applyConfig(patch, true, errors)) LOG_E(errors.dump());
    }

    json _Livox1::configValues() const
    {
        return {{"lvxSN", m_lvxSN}, {"lvxIP", ipString(m_lvxIP)}, {"lvxHostIP", ipString(m_lvxCfg.m_hostIP)},
            {"lvxPortBroadcast", m_broadcastPort}, {"lvxPortCmd", m_lvxCmdPort}, {"lvxHostPortCmd", m_hostPortCmd},
            {"lvxHostPortPCL", m_lvxCfg.m_hostPortPCL}, {"lvxHostPortIMU", m_lvxCfg.m_hostPortIMU},
            {"receiveBufferBytes", m_receiveBufferBytes}, {"lvxCoordinate", m_lvxCfg.m_coordinate},
            {"lvxReturnMode", m_lvxCfg.m_returnMode}, {"lvxScanPattern", m_lvxCfg.m_scanPattern},
            {"lvxWorkMode", m_lvxCfg.m_workMode}, {"lvxIMUdataEn", m_lvxCfg.m_imuDataEn},
            {"lvxSampling", m_lvxCfg.m_sampling}, {"lvxFan", m_lvxCfg.m_fan}, {"lvxHighSensitivity", m_lvxCfg.m_highSensitivity},
            {"bIMUpreview", m_bIMUpreview}, {"bIMUstab", m_bIMUstab}, {"bReflectivity", m_bReflectivity},
            {"frameDurationMs", m_frameDurationNs / NSEC_MSEC}, {"tOutSec", m_timeoutNs / NSEC_SEC},
            {"nMaxFramePoints", m_nMaxFramePoints}, {"vColorDefault", {m_vColorDefault.x(), m_vColorDefault.y(), m_vColorDefault.z()}},
            {"vRangeD", {m_vRangeD.x(), m_vRangeD.y()}}};
    }

    json _Livox1::controlSchema() const
    {
        json schema = json::object();
        const auto choices = [](std::initializer_list<std::pair<int, const char *>> values) {
            json result = json::array();
            for (const auto &value : values) result.push_back({{"value", value.first}, {"label", value.second}});
            return result;
        };
        schema["lvxCoordinate"] = {{"type", "integer"}, {"label", "Coordinates"}, {"choices", choices({{0, "Cartesian"}, {1, "Spherical"}})}};
        schema["lvxReturnMode"] = {{"type", "integer"}, {"label", "Return mode"}, {"choices", choices({{0, "First"}, {1, "Strongest"}, {2, "Dual"}, {3, "Triple (Avia)"}})}};
        if (m_lvxDevType != livox1::Unknown && m_lvxDevType != livox1::Avia) schema["lvxReturnMode"]["choices"].erase(3);
        schema["lvxScanPattern"] = {{"type", "integer"}, {"label", "Scan pattern"}, {"choices", choices({{0, "Non-repetitive"}, {1, "Repetitive (Avia)"}})}};
        schema["lvxWorkMode"] = {{"type", "integer"}, {"label", "Work mode"}, {"choices", choices({{1, "Normal"}, {2, "Power saving"}, {3, "Standby"}})}};
        schema["lvxIMUdataEn"] = {{"type", "integer"}, {"label", "IMU output"}, {"choices", choices({{0, "Disabled"}, {1, "200 Hz"}})}};
        schema["lvxSampling"] = {{"type", "bool"}, {"label", "Stream point cloud"}, {"description", "Starts sampling when work mode is Normal"}};
        schema["lvxFan"] = {{"type", "bool"}, {"label", "Cooling fan"}, {"description", "Supported by Mid-40, Horizon and Tele-15 firmware with fan control"}};
        schema["lvxHighSensitivity"] = {{"type", "bool"}, {"label", "High sensitivity"}, {"description", "High sensitivity detection on Avia and Tele-15"}};
        for (const char *key : {"lvxReturnMode", "lvxScanPattern", "lvxIMUdataEn", "lvxFan", "lvxHighSensitivity"})
        {
            schema[key]["supported"] = supportsSetting(key);
            if (!supportsSetting(key))
            { schema[key]["readOnly"] = true; schema[key]["description"] = "Unsupported by the detected model/firmware; ignored"; }
        }
        schema["frameDurationMs"] = {{"type", "integer"}, {"label", "Cloud frame duration (ms)"}, {"min", 20}, {"max", 1000}, {"description", "Host grouping interval; legacy packets do not contain frame boundaries"}};
        schema["tOutSec"] = {{"type", "integer"}, {"label", "Disconnect timeout (s)"}, {"min", 2}, {"max", 120}};
        schema["nMaxFramePoints"] = {{"type", "integer"}, {"label", "Maximum points per frame"}, {"min", 100}, {"max", 1000000}};
        schema["bIMUpreview"] = {{"type", "bool"}, {"label", "Enable IMU preview"}, {"description", "Compute viewer orientation and graphs; raw IMUstream output is independent"}};
        schema["bIMUstab"] = {{"type", "bool"}, {"label", "Stabilize cloud roll/pitch"}, {"description", "Requires IMU preview. Disable for SLAM, which performs its own fusion"}};
        schema["bReflectivity"] = {{"type", "bool"}, {"label", "Reflectivity coloring"}};
        schema["vColorDefault"] = {{"type", "array"}, {"label", "Default RGB color"}, {"length", 3}, {"min", 0}, {"max", 1}};
        schema["vRangeD"] = {{"type", "range"}, {"label", "Distance range (m)"}, {"length", 2}, {"min", 0}, {"max", 10000}};
        for (const char *key : {"lvxSN", "lvxIP", "lvxHostIP"})
            schema[key] = {{"type", "string"}, {"readOnly", true}, {"category", "Network"}, {"description", "Edit this Livox instance in JSON and restart"}};
        schema["lvxSN"]["description"] = "Optional broadcast code filter (up to 15 characters)";
        schema["lvxHostIP"]["description"] = "Local IPv4 address; 0.0.0.0 selects the route to lvxIP";
        for (const char *key : {"lvxPortBroadcast", "lvxPortCmd", "lvxHostPortCmd", "lvxHostPortPCL", "lvxHostPortIMU"})
            schema[key] = {{"type", "integer"}, {"min", 1}, {"max", 65535}, {"readOnly", true}, {"category", "Network"},
                {"description", "UDP port; edit this Livox instance in JSON and restart"}};
        schema["receiveBufferBytes"] = {{"type", "integer"}, {"min", 65536}, {"max", 67108864}, {"readOnly", true},
            {"category", "Network"}, {"description", "Requested kernel receive buffer per socket (bytes); restart to apply"}};
        return schema;
    }

    bool _Livox1::applyConfig(const json &patch, bool live, json &errors)
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
            { errors[key] = spec.value("supported", true) ? "Edit the JSON configuration and restart" : "Unsupported by the detected model/firmware"; continue; }
            bool valid = true;
            if (key == "lvxSN") valid = value.is_string() && value.get<string>().size() <= 15;
            else if (key == "lvxIP" || key == "lvxHostIP")
            {
                uint32_t address = 0;
                valid = readIP(value, address) && (key == "lvxHostIP" || address != 0) &&
                    (address == 0 || (ntohl(address) >> 24) > 0) && (ntohl(address) >> 24) < 224;
            }
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
        const char *ports[] = {"lvxPortBroadcast", "lvxHostPortCmd", "lvxHostPortPCL", "lvxHostPortIMU"};
        for (size_t i = 0; i < 4; ++i)
            for (size_t j = 0; j < i; ++j)
                if (next.at(ports[i]) == next.at(ports[j])) errors[ports[i]] = "Local UDP ports must be distinct";
        if (next.at("bIMUstab").get<bool>() && !next.at("bIMUpreview").get<bool>())
            errors["bIMUstab"] = "bIMUstab requires bIMUpreview=true; enable preview or disable stabilization";
        if (!errors.empty()) return false;
        m_lvxSN = next.at("lvxSN");
        readIP(next.at("lvxIP"), m_lvxIP); readIP(next.at("lvxHostIP"), m_lvxCfg.m_hostIP);
        m_broadcastPort = next.at("lvxPortBroadcast"); m_lvxCmdPort = next.at("lvxPortCmd");
        m_hostPortCmd = next.at("lvxHostPortCmd"); m_receiveBufferBytes = next.at("receiveBufferBytes");
        m_lvxCfg.m_hostPortPCL = next.at("lvxHostPortPCL"); m_lvxCfg.m_hostPortIMU = next.at("lvxHostPortIMU");
        m_lvxCfg.m_coordinate = next.at("lvxCoordinate"); m_lvxCfg.m_returnMode = next.at("lvxReturnMode");
        m_lvxCfg.m_scanPattern = next.at("lvxScanPattern"); m_lvxCfg.m_workMode = next.at("lvxWorkMode");
        m_lvxCfg.m_imuDataEn = next.at("lvxIMUdataEn"); m_lvxCfg.m_sampling = next.at("lvxSampling"); m_lvxCfg.m_fan = next.at("lvxFan");
        m_lvxCfg.m_highSensitivity = next.at("lvxHighSensitivity");
        m_bIMUpreview = next.at("bIMUpreview"); m_bIMUstab = next.at("bIMUstab"); m_bReflectivity = next.at("bReflectivity");
        m_frameDurationNs = next.at("frameDurationMs").get<uint64_t>() * NSEC_MSEC;
        m_timeoutNs = next.at("tOutSec").get<uint64_t>() * NSEC_SEC;
        m_nMaxFramePoints = next.at("nMaxFramePoints");
        for (int i = 0; i < 3; ++i) m_vColorDefault[i] = next.at("vColorDefault")[i];
        for (int i = 0; i < 2; ++i) m_vRangeD[i] = next.at("vRangeD")[i];
        if (before != next)
        {
            // Preview collection does not alter point decoding or the current cloud frame.
            for (const auto &field : before.items())
                if (field.key() != "bIMUpreview" && field.value() != next.at(field.key()))
                { resetFrame(); break; }
            if (m_bIMUpreview && (before.at("bIMUpreview") != next.at("bIMUpreview") ||
                before.at("lvxIMUdataEn") != next.at("lvxIMUdataEn"))) m_imuPreview.reset();
            if (before.at("lvxIMUdataEn") != next.at("lvxIMUdataEn"))
            {
                m_tIMU = 0;
                if (live && m_pIMUout)
                {
                    m_pIMUout->clear();
                }
            }
            if (live && !m_bIMUstab && before.at("bIMUstab") != next.at("bIMUstab"))
            {
                // Restore the configured mounting transform after preview stabilization.
                if (m_pTf)
                {
                    m_pTf->setRotation(m_mountOrientation, true);
                }
            }
        }
        if (live && m_state != State::Discovering && m_state != State::Handshaking && m_state != State::Initializing)
        {
            bool changed = false;
            for (const char *key : {"lvxCoordinate", "lvxReturnMode", "lvxScanPattern", "lvxWorkMode", "lvxIMUdataEn", "lvxSampling", "lvxFan", "lvxHighSensitivity"})
                changed |= before.at(key) != next.at(key);
            if (changed || (m_configFailed && !patch.empty()))
            {
                // Cancel obsolete setting transactions before issuing a new
                // configuration. Old sequence numbers can no longer ACK newer
                // desired values or resend superseded payloads.
                for (auto it = m_pending.begin(); it != m_pending.end();)
                    if (it->second.setting) it = m_pending.erase(it); else ++it;
                m_lastError.clear(); configureDevice();
            }
        }
        return true;
    }

    json _Livox1::imuValues() const
    {
        IMUpreview::Snapshot sample;
        if (m_bIMUpreview) sample = m_imuPreview.snapshot();
        const uint64_t now = getTns();
        const bool open = m_state != State::Discovering && m_state != State::Handshaking;
        const bool enabled = m_lvxCfg.m_imuDataEn && supportsSetting("lvxIMUdataEn");
        const bool available = m_bIMUpreview && open && enabled && sample.tGyro && m_lastIMUrx && now - m_lastIMUrx < NSEC_SEC / 2;
        return {{"enabled", enabled}, {"previewEnabled", m_bIMUpreview}, {"deviceOpen", open}, {"available", available},
            {"tGyro", std::to_string(sample.tGyro)}, {"tAcc", std::to_string(sample.tAcc)}, {"tFusion", std::to_string(sample.tFusion)},
            {"gyro", {sample.gyro.x(), sample.gyro.y(), sample.gyro.z()}}, {"acc", {sample.acc.x(), sample.acc.y(), sample.acc.z()}},
            {"quaternion", {sample.quaternion.w(), sample.quaternion.x(), sample.quaternion.y(), sample.quaternion.z()}},
            {"rpy", {sample.rpy.x(), sample.rpy.y(), sample.rpy.z()}}, {"fusion", true}, {"orientationValid", available && sample.orientationValid}};
    }

    void _Livox1::console(void *console)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        _ModuleBase::console(console);
        if (console) static_cast<_Console *>(console)->addMsg("Livox1 state=" + std::to_string(int(m_state)) + " SN=" + m_detectedSN +
            " frames=" + std::to_string(m_frames) + " IMU=" + std::to_string(m_imuSamples) + " " + m_lastError);
    }

    void _Livox1::jsonCmd(const json &request, void *pJSONbase)
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
                for (const auto &entry : m_pending) setting += entry.second.setting;
                const bool open = m_state != State::Discovering && m_state != State::Handshaking;
                const char *state = m_state == State::Discovering ? "discovering" : m_state == State::Handshaking ? "handshaking" :
                    m_state == State::Initializing ? "initializing" : m_state == State::Configuring ? "configuring" : "connected";
                reply["config"] = configValues(); reply["schema"] = controlSchema(); reply["deviceOpen"] = open;
                reply["status"] = {{"state", state}, {"serialNumber", m_detectedSN}, {"deviceType", m_lvxDevType},
                    {"commandPort", m_lvxCmdPort}, {"hostIP", ipString(m_hostIP)}, {"socketsOpen", m_started},
                    {"pointPackets", m_pointPackets}, {"points", m_points}, {"frames", m_frames}, {"imuSamples", m_imuSamples},
                    {"invalidPackets", m_invalidPackets}, {"outOfOrderPackets", m_outOfOrderPackets}, {"frameOverflows", m_frameOverflows},
                    {"commandErrors", m_commandErrors}, {"commandRetries", m_commandRetries}, {"pendingCommands", m_pending.size()},
                    {"pendingSettings", setting},
                    {"configApplied", m_state == State::Connected && !setting && !m_configFailed},
                    {"lastError", m_lastError}, {"observed", m_observed}};
                if (!m_lastError.empty()) reply["error"] = m_lastError;
            }
        }
        catch (const std::exception &e) { reply["bSuccess"] = false; reply["error"] = e.what(); }
        endpoint->sendJson(reply);
    }
}
