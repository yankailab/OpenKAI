#include "_E1R.h"
#include "../../../../Protocol/_JSONbase.h"
#include <poll.h>
#include <limits>

namespace kai
{
    namespace
    {
        bool validIP(const json &value)
        {
            in_addr address{};
            return value.is_string() && inet_pton(AF_INET, value.get<string>().c_str(), &address) == 1;
        }

        int openUDP(const string &bindIP, int port, int bufferBytes, string &error)
        {
            const int fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd < 0) { error = "UDP socket: " + string(strerror(errno)); return -1; }
            int reuse = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0 ||
                setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufferBytes, sizeof(bufferBytes)) < 0 ||
                fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
            {
                error = "UDP socket options: " + string(strerror(errno)); close(fd); return -1;
            }
            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_port = htons(static_cast<uint16_t>(port));
            inet_pton(AF_INET, bindIP.c_str(), &address.sin_addr);
            if (bind(fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
            {
                error = "Bind " + bindIP + ":" + std::to_string(port) + ": " + strerror(errno);
                close(fd); return -1;
            }
            return fd;
        }

        // Expose every documented DIFOP parameter without inventing an input
        // protocol: the E1R user guide specifies MSOP and DIFOP as output-only.
        const std::pair<const char *, const char *> deviceFields[] = {
            {"deviceSensorIP", "sensorIp"}, {"deviceNetmask", "netmask"}, {"deviceMAC", "mac"},
            {"deviceMSOPip", "msopIp"}, {"deviceMSOPlocalPort", "msopLocalPort"}, {"deviceMSOPport", "msopPort"},
            {"deviceDIFOPip", "difopIp"}, {"deviceDIFOPlocalPort", "difopLocalPort"}, {"deviceDIFOPport", "difopPort"},
            {"deviceFrameRate", "frameRate"}, {"deviceReturnMode", "returnMode"}, {"deviceTimeMode", "timeMode"},
            {"deviceTimeStatus", "timeStatus"}, {"devicePHYmode", "phyMode"},
            {"deviceSerial", "serial"}, {"deviceFirmware", "firmware"}};
    }

    _E1R::_E1R()
    {
        m_config = {{"deviceIP", "192.168.1.200"}, {"bindIP", "0.0.0.0"},
            {"msopPort", 6699}, {"difopPort", 7788}, {"receiveBufferBytes", 4194304},
            {"tOutSec", 3}, {"frameTimeoutMs", 250}, {"nMaxFramePoints", 50000},
            {"bReflectivity", true}, {"bDropNoise", true}, {"bUseSensorTime", true},
            {"vRangeD", {0.1, 120.0}}, {"vColorDefault", {1.0, 1.0, 1.0}},
            {"vPos", {0.0, 0.0, 0.0}}, {"vOrt", {0.0, 0.0, 0.0, 1.0}}};
    }

    _E1R::~_E1R() { stop(); }

    bool _E1R::loadConfig()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        IF_F(!_TransformBase::loadConfig());
        json patch = json::object(), errors;
        for (const auto &field : m_config.items())
            if (m_pJ->contains(field.key())) patch[field.key()] = m_pJ->at(field.key());
        if (!applyConfig(patch, false, errors)) { LOG_E(errors.dump()); return false; }
        m_framePoints.reserve(m_config.at("nMaxFramePoints").get<size_t>());
        return true;
    }

    bool _E1R::saveConfig(bool bExport)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        IF_F(!_TransformBase::saveConfig(false));
        m_pJ->update(m_config);
        return !bExport || m_pJcfg->saveToFile();
    }

    bool _E1R::link(InstanceMgr *pM)
    {
        IF_F(!_TransformBase::link(pM));
        string name;
        jKv(*m_pJ, "PCLframeOut", name);
        m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(name)));
        IF_Le_F(!m_pPCLout, "PCLframeOut not found: " + name);
        return true;
    }

    bool _E1R::check() { return m_pPCLout && _TransformBase::check(); }

    bool _E1R::openSockets(const json &config, string &error)
    {
        const string ip = config.at("bindIP");
        const int buffer = config.at("receiveBufferBytes");
        const int msop = openUDP(ip, config.at("msopPort"), buffer, error);
        if (msop < 0) return false;
        const int difop = openUDP(ip, config.at("difopPort"), buffer, error);
        if (difop < 0) { close(msop); return false; }
        closeSockets();
        m_msopSocket = msop;
        m_difopSocket = difop;
        ++m_socketGeneration;
        return true;
    }

    void _E1R::closeSockets()
    {
        if (m_msopSocket >= 0) close(m_msopSocket);
        if (m_difopSocket >= 0) close(m_difopSocket);
        m_msopSocket = m_difopSocket = -1;
        ++m_socketGeneration;
    }

    bool _E1R::start()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (!check()) return false;
        if (m_started) return true;
        if (!openSockets(m_config, m_lastError)) { LOG_E(m_lastError); return false; }
        resetFrame();
        m_lastMSOP = m_lastDIFOP = 0;
        m_selectedIP.clear();
        m_observed = json::object();
        m_timedOut = false;
        m_started = true;
        m_lastError.clear();
        if (m_pT->startThread(getUpdate, this)) return true;
        m_started = false;
        closeSockets();
        m_lastError = "Could not start E1R receive thread";
        return false;
    }

    void _E1R::stop()
    {
        // Do not hold the state lock while joining a worker that needs it.
        if (m_pT) { m_pT->stop(); m_pT->join(); }
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        closeSockets();
        m_started = false;
        resetFrame();
    }

    void _E1R::update()
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();
            if (!m_pT->bRun()) break;
            pollfd sockets[2]{};
            uint64_t generation;
            {
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
                sockets[0] = {m_msopSocket, POLLIN, 0};
                sockets[1] = {m_difopSocket, POLLIN, 0};
                generation = m_socketGeneration;
            }
            const int ready = poll(sockets, 2, 20);
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            // A web configuration update can replace both sockets during poll.
            if (generation != m_socketGeneration) continue;
            if (ready < 0 && errno != EINTR) m_lastError = "UDP poll: " + string(strerror(errno));
            if (ready > 0)
                for (int i = 0; i < 2; ++i)
                {
                    if (sockets[i].revents & POLLIN) receive(sockets[i].fd, i == 0);
                    if (sockets[i].revents & (POLLERR | POLLNVAL)) m_lastError = "E1R UDP receive socket error";
                }
            const uint64_t now = getTns();
            if (m_frameActive && m_lastMSOP && now - m_lastMSOP >= m_config.at("frameTimeoutMs").get<uint64_t>() * NSEC_MSEC)
                publishFrame(true);
            const uint64_t last = std::max(m_lastMSOP, m_lastDIFOP);
            if (last && now - last >= m_config.at("tOutSec").get<uint64_t>() * NSEC_SEC && !m_timedOut)
            {
                m_timedOut = true;
                resetFrame();
                if (m_config.at("deviceIP") == "0.0.0.0") m_selectedIP.clear();
            }
        }
    }

    void _E1R::receive(int fd, bool msop)
    {
        // Bound each batch so DIFOP, stop and console edits cannot starve.
        for (unsigned i = 0; i < 512; ++i)
        {
            uint8_t bytes[2048];
            sockaddr_in source{};
            socklen_t sourceSize = sizeof(source);
            const ssize_t length = recvfrom(fd, bytes, sizeof(bytes), MSG_TRUNC,
                reinterpret_cast<sockaddr *>(&source), &sourceSize);
            if (length < 0)
            {
                if (errno == EINTR) continue;
                if (errno != EAGAIN && errno != EWOULDBLOCK) m_lastError = "UDP receive: " + string(strerror(errno));
                break;
            }
            char address[INET_ADDRSTRLEN]{};
            inet_ntop(AF_INET, &source.sin_addr, address, sizeof(address));
            const string filter = m_config.at("deviceIP");
            if ((filter != "0.0.0.0" && filter != address) || (!m_selectedIP.empty() && m_selectedIP != address)) continue;
            string error;
            const uint64_t now = getTns();
            bool valid = false;
            if (length <= static_cast<ssize_t>(sizeof(bytes)))
            {
                if (msop)
                {
                    e1r::Msop packet;
                    if ((valid = e1r::decodeMsop(bytes, static_cast<size_t>(length), packet, &error))) handleMSOP(packet, now);
                }
                else
                {
                    e1r::Difop packet;
                    if ((valid = e1r::decodeDifop(bytes, static_cast<size_t>(length), packet, &error))) handleDIFOP(packet, now);
                }
            }
            if (!valid) { ++m_invalidPackets; continue; }
            m_selectedIP = address;
            m_timedOut = false;
        }
    }

    void _E1R::resetFrame()
    {
        m_framePoints.clear();
        m_frameActive = false;
        m_framePartial = false;
        m_haveSequence = false;
        m_frameStamp = 0;
        m_lastPacketUs = 0;
    }

    void _E1R::publishFrame(bool partial)
    {
        if (!m_frameActive) return;
        // PCL consumers use strictly increasing frame IDs even after a sensor
        // clock reset. Individual point timestamps retain the chosen time base.
        const uint64_t stamp = std::max(m_frameStamp, m_lastPublishedStamp + 1);
        if (m_pPCLout) m_pPCLout->set(m_framePoints, stamp);
        m_lastPublishedStamp = stamp;
        ++m_frames;
        if (partial || m_framePartial) ++m_partialFrames;
        m_framePoints.clear();
        m_frameActive = false;
        m_framePartial = false;
    }

    void _E1R::handleMSOP(const e1r::Msop &packet, uint64_t receivedNs)
    {
        const bool sensorTime = m_config.at("bUseSensorTime");
        const uint64_t maxNs = std::numeric_limits<uint64_t>::max();
        const uint64_t maxOffsetNs = uint64_t(std::numeric_limits<uint16_t>::max()) * 1000;
        if ((sensorTime && packet.timestampUs > (maxNs - maxOffsetNs) / 1000) ||
            (!sensorTime && receivedNs > maxNs - maxOffsetNs))
        { ++m_invalidPackets; return; }
        ++m_pointPackets;
        m_lastMSOP = receivedNs;
        m_observed["temperatureC"] = packet.temperature;
        m_observed["frameSync"] = packet.frameSync;
        m_observed["protocolVersion"] = packet.version;
        m_observed["msopReturnMode"] = packet.returnMode;
        m_observed["msopTimeMode"] = packet.timeMode;
        const bool clockReset = m_haveSequence && (packet.timeMode != m_timeMode ||
            (m_lastPacketUs > packet.timestampUs && m_lastPacketUs - packet.timestampUs >= 1000000));
        if (clockReset) { publishFrame(true); resetFrame(); ++m_clockResets; }
        bool newSequenceFrame = !m_haveSequence;
        if (m_haveSequence)
        {
            const bool firstPacket = packet.sequence == 0 && packet.timestampUs > m_lastPacketUs;
            if (packet.timestampUs < m_lastPacketUs || (packet.sequence == m_lastSequence && !firstPacket) ||
                (packet.sequence < m_lastSequence && packet.timestampUs == m_lastPacketUs))
            { ++m_outOfOrderPackets; return; }
            if (packet.sequence < m_lastSequence || firstPacket) { publishFrame(); newSequenceFrame = true; }
            else if (packet.sequence > unsigned(m_lastSequence) + 1)
            {
                m_droppedPackets += packet.sequence - unsigned(m_lastSequence) - 1;
                m_framePartial = true;
            }
        }
        // Validate conversion to OpenKAI nanoseconds separately from the
        // protocol parser, which retains the full microsecond wire timestamp.
        const uint64_t packetNs = sensorTime ? packet.timestampUs * 1000 : receivedNs;
        if (!m_frameActive)
        {
            m_frameActive = true;
            m_frameStamp = std::max<uint64_t>(packetNs, 1);
            m_framePartial = m_framePartial || !newSequenceFrame || packet.sequence != 0;
            if (newSequenceFrame && packet.sequence != 0) m_droppedPackets += packet.sequence;
        }
        m_haveSequence = true;
        m_lastPacketUs = packet.timestampUs;
        m_lastSequence = packet.sequence;
        m_timeMode = packet.timeMode;
        const float minimum = m_config.at("vRangeD")[0], maximum = m_config.at("vRangeD")[1];
        const size_t limit = m_config.at("nMaxFramePoints");
        const bool reflectivity = m_config.at("bReflectivity"), dropNoise = m_config.at("bDropNoise");
        const Vector3f color(m_config.at("vColorDefault")[0], m_config.at("vColorDefault")[1], m_config.at("vColorDefault")[2]);
        bool overflow = false;
        for (const auto &sample : packet.points)
        {
            // The guide defines 1 as normal and 2 as noise. Live E1R firmware
            // 8.1.1 emits 0 for every point, including valid nonzero ranges;
            // retain these unclassified points as well as documented normals.
            if (sample.range <= 0 || sample.range < minimum || sample.range > maximum ||
                (dropNoise && sample.attribute > 1)) continue;
            if (m_framePoints.size() >= limit) { overflow = true; continue; }
            GEOMETRY_POINT point;
            point.m_vP = m_mPosef * Vector3f(sample.x, sample.y, sample.z);
            point.m_vC = reflectivity ? Vector3f::Constant(sample.intensity / 255.0f) : color;
            point.m_tStamp = std::max<uint64_t>(packetNs + uint64_t(sample.offsetUs) * 1000, 1);
            m_framePoints.push_back(point);
            ++m_points;
        }
        if (overflow) { ++m_frameOverflows; m_framePartial = true; }
    }

    void _E1R::handleDIFOP(const e1r::Difop &packet, uint64_t receivedNs)
    {
        ++m_difopPackets;
        m_lastDIFOP = receivedNs;
        m_observed.update({{"sensorIp", packet.sensorIp}, {"netmask", packet.netmask}, {"mac", packet.mac},
            {"msopIp", packet.msopIp}, {"msopLocalPort", packet.msopLocalPort}, {"msopPort", packet.msopPort},
            {"difopIp", packet.difopIp}, {"difopLocalPort", packet.difopLocalPort}, {"difopPort", packet.difopPort},
            {"serial", packet.serial}, {"firmware", packet.firmware}, {"frameRate", packet.frameRate},
            {"returnMode", packet.returnMode}, {"timeMode", packet.timeMode}, {"timeStatus", packet.timeStatus},
            {"phyMode", packet.phyMode}, {"timestampUs", std::to_string(packet.timestampUs)}});
    }

    void _E1R::clear()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        resetFrame();
        if (m_pPCLout) m_pPCLout->set({}, ++m_lastPublishedStamp);
    }

    json _E1R::configValues() const
    {
        json values = m_config;
        for (const auto &field : deviceFields) values[field.first] = m_observed.value(field.second, json());
        return values;
    }

    json _E1R::controlSchema() const
    {
        json schema = json::object();
        schema["deviceIP"] = {{"type", "string"}, {"label", "LiDAR source IPv4"}, {"category", "Network"},
            {"description", "192.168.1.200 is the factory address; 0.0.0.0 accepts the first valid E1R sender. This is a host filter and does not change the sensor address."}};
        schema["bindIP"] = {{"type", "string"}, {"label", "Host bind IPv4"}, {"category", "Network"},
            {"description", "0.0.0.0 listens on all host interfaces. The host must own the LiDAR destination address (factory default 192.168.1.102)."}};
        for (const char *key : {"msopPort", "difopPort"})
            schema[key] = {{"type", "integer"}, {"label", key == string("msopPort") ? "Host MSOP UDP port" : "Host DIFOP UDP port"},
                {"category", "Network"}, {"min", 1}, {"max", 65535}, {"description", "Host listen port; does not change the sensor destination port."}};
        schema["receiveBufferBytes"] = {{"type", "integer"}, {"label", "UDP receive buffer (bytes)"}, {"category", "Network"}, {"min", 65536}, {"max", 67108864},
            {"description", "Requested socket buffer; the operating system may cap this value."}};
        schema["tOutSec"] = {{"type", "integer"}, {"label", "Disconnect timeout (s)"}, {"category", "Network"}, {"min", 1}, {"max", 120}};
        schema["frameTimeoutMs"] = {{"type", "integer"}, {"label", "Partial frame timeout (ms)"}, {"category", "Point cloud"}, {"min", 20}, {"max", 2000},
            {"description", "Publish the remaining partial frame after this interval without MSOP packets. Normal frames follow the packet sequence reset."}};
        schema["nMaxFramePoints"] = {{"type", "integer"}, {"label", "Maximum points per frame"}, {"category", "Point cloud"}, {"min", 96}, {"max", 1000000}};
        schema["bReflectivity"] = {{"type", "bool"}, {"label", "Reflectivity coloring"}, {"category", "Point cloud"}};
        schema["bDropNoise"] = {{"type", "bool"}, {"label", "Filter noisy point attributes"}, {"category", "Point cloud"},
            {"description", "Keep normal (1) and unclassified (0, used by firmware 8.1.1) points; reject noise (2) and unknown attributes. Zero ranges are always discarded."}};
        schema["bUseSensorTime"] = {{"type", "bool"}, {"label", "Use sensor timestamps"}, {"category", "Point cloud"},
            {"description", "When disabled, timestamp packets with host monotonic receive time, retaining per-point offsets."}};
        schema["vRangeD"] = {{"type", "range"}, {"label", "Distance range (m)"}, {"category", "Point cloud"}, {"length", 2}, {"min", 0}, {"max", 327.675}};
        schema["vColorDefault"] = {{"type", "array"}, {"label", "Default RGB color"}, {"category", "Point cloud"}, {"length", 3}, {"min", 0}, {"max", 1}};
        schema["vPos"] = {{"type", "array"}, {"label", "Mount position XYZ (m)"}, {"category", "Mount transform"}, {"length", 3}, {"min", -1000000}, {"max", 1000000}};
        schema["vOrt"] = {{"type", "array"}, {"label", "Mount quaternion XYZW"}, {"category", "Mount transform"}, {"length", 4}, {"min", -1}, {"max", 1},
            {"description", "Normalized on apply. Identity is [0, 0, 0, 1]."}};
        for (const auto &field : deviceFields)
            schema[field.first] = {{"type", "string"}, {"label", field.second}, {"category", "Sensor (DIFOP)"}, {"readOnly", true},
                {"description", "Observed sensor value. The supplied E1R manual defines output-only DIFOP; a supported device write protocol was not supplied."}};
        return schema;
    }

    bool _E1R::applyConfig(const json &patch, bool live, json &errors)
    {
        errors = json::object();
        if (!patch.is_object()) { errors["config"] = "Expected an object"; return false; }
        const json schema = controlSchema(), values = configValues();
        json next = m_config;
        for (const auto &field : patch.items())
        {
            if (!schema.contains(field.key())) { errors[field.key()] = "Unknown parameter"; continue; }
            const json &spec = schema.at(field.key()), &value = field.value();
            if (spec.value("readOnly", false))
            {
                if (value != values.at(field.key())) errors[field.key()] = "Sensor parameter is read-only: no supported E1R device write protocol was supplied";
                continue;
            }
            bool valid = true;
            const string type = spec.at("type");
            if (type == "string") valid = validIP(value);
            else if (type == "bool") valid = value.is_boolean();
            else if (type == "array" || type == "range")
            {
                valid = value.is_array() && value.size() == spec.at("length").get<size_t>();
                if (valid)
                    for (const auto &element : value)
                        if (!element.is_number() || !std::isfinite(element.get<double>()) ||
                            element.get<double>() < spec.at("min").get<double>() || element.get<double>() > spec.at("max").get<double>()) valid = false;
                if (valid && field.key() == "vRangeD") valid = value[0].get<double>() < value[1].get<double>();
                if (valid && field.key() == "vOrt")
                {
                    double norm = 0;
                    for (const auto &element : value) norm += element.get<double>() * element.get<double>();
                    valid = norm > 1e-12;
                }
            }
            else valid = value.is_number_integer() && value.get<double>() >= spec.at("min").get<double>() && value.get<double>() <= spec.at("max").get<double>();
            if (!valid) { errors[field.key()] = "Invalid type or value outside supported range"; continue; }
            next[field.key()] = value;
        }
        if (next.at("msopPort") == next.at("difopPort")) errors["difopPort"] = "MSOP and DIFOP host ports must differ";
        if (!errors.empty()) return false;
        Quaterniond orientation(next.at("vOrt")[3].get<double>(), next.at("vOrt")[0].get<double>(),
            next.at("vOrt")[1].get<double>(), next.at("vOrt")[2].get<double>());
        orientation.normalize();
        next["vOrt"] = {orientation.x(), orientation.y(), orientation.z(), orientation.w()};
        bool networkChanged = false;
        for (const char *key : {"bindIP", "msopPort", "difopPort", "receiveBufferBytes"}) networkChanged |= next.at(key) != m_config.at(key);
        if (live && m_started && networkChanged)
        {
            string error;
            if (!openSockets(next, error)) { errors["network"] = error; return false; }
        }
        if (next != m_config)
        {
            resetFrame();
            if (networkChanged || next.at("deviceIP") != m_config.at("deviceIP"))
            {
                m_selectedIP.clear(); m_lastMSOP = m_lastDIFOP = 0;
                m_observed = json::object(); m_timedOut = false; m_lastError.clear();
            }
            m_config = next;
        }
        setPos(Vector3d(next.at("vPos")[0], next.at("vPos")[1], next.at("vPos")[2]));
        setOrientation(orientation, true);
        return true;
    }

    json _E1R::statusValues() const
    {
        const uint64_t now = getTns(), last = std::max(m_lastMSOP, m_lastDIFOP);
        const uint64_t timeout = m_config.at("tOutSec").get<uint64_t>() * NSEC_SEC;
        const bool connected = m_started && last && now - last < timeout;
        const bool streaming = connected && m_lastMSOP && now - m_lastMSOP < timeout;
        const string state = !m_lastError.empty() ? "error" : !m_started ? "stopped" : streaming ? "streaming" :
            connected ? "device information only" : last ? "disconnected" : "waiting";
        return {{"state", state}, {"deviceOpen", connected}, {"sourceIP", m_selectedIP},
            {"pointPackets", m_pointPackets}, {"difopPackets", m_difopPackets}, {"points", m_points},
            {"frames", m_frames}, {"partialFrames", m_partialFrames}, {"droppedPackets", m_droppedPackets},
            {"invalidPackets", m_invalidPackets}, {"outOfOrderPackets", m_outOfOrderPackets},
            {"frameOverflows", m_frameOverflows}, {"clockResets", m_clockResets},
            {"lastPacketAgeMs", last ? json((now - last) / NSEC_MSEC) : json()},
            {"lastMSOPageMs", m_lastMSOP ? json((now - m_lastMSOP) / NSEC_MSEC) : json()},
            {"lastDIFOPageMs", m_lastDIFOP ? json((now - m_lastDIFOP) / NSEC_MSEC) : json()},
            {"deviceControlSupported", false}, {"lastError", m_lastError}, {"observed", m_observed}};
    }

    void _E1R::console(void *pConsole)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        _TransformBase::console(pConsole);
        if (pConsole) static_cast<_Console *>(pConsole)->addMsg("E1R " + statusValues().at("state").get<string>() +
            " " + m_selectedIP + " frames=" + std::to_string(m_frames) + " " + m_lastError);
    }

    void _E1R::console(const json &request, void *pJSONbase)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto *endpoint = static_cast<_JSONbase *>(pJSONbase);
        if (!endpoint || !request.is_object() || !request.contains("cmd") || !request.at("cmd").is_string()) return;
        const string command = request.at("cmd");
        const bool set = command == "setConfig" || command == "config.set";
        const bool save = command == "saveConfig" || command == "config.save";
        if (!set && !save && command != "getConfig" && command != "config.get" && command != "config.schema" && command != "status") return;
        json reply = {{"cmd", command}, {"module", getName()}, {"bSuccess", true}};
        if (request.contains("requestId")) reply["requestId"] = request.at("requestId");
        try
        {
            if (set)
            {
                json errors;
                reply["bSuccess"] = applyConfig(request.value("config", json()), true, errors);
                reply["errors"] = errors;
            }
            else if (save) reply["bSuccess"] = saveConfig(true);
            reply["config"] = configValues();
            reply["schema"] = controlSchema();
            reply["status"] = statusValues();
            reply["deviceOpen"] = reply["status"]["deviceOpen"];
            if (!m_lastError.empty()) reply["error"] = m_lastError;
        }
        catch (const std::exception &e) { reply["bSuccess"] = false; reply["error"] = e.what(); }
        endpoint->sendJson(reply);
    }
}
