#include "_Airy.h"
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

        // Device settings are observed through DIFOP. The Airy sensor's web
        // interface is the documented configuration interface, not DIFOP input.
        const std::pair<const char *, const char *> deviceFields[] = {
            {"deviceSensorIP", "sensorIp"}, {"deviceDestinationIP", "destinationIp"}, {"deviceMAC", "mac"},
            {"deviceMSOPport", "msopPort"}, {"deviceDIFOPport", "difopPort"},
            {"deviceRPM", "rpm"}, {"deviceMotorSpeed", "motorSpeed"}, {"deviceReturnMode", "returnMode"},
            {"deviceTimeMode", "timeMode"}, {"deviceTimeStatus", "timeStatus"}, {"deviceSerial", "serial"},
            {"deviceMainFirmware", "mainFirmware"}, {"deviceBaseFirmware", "baseFirmware"},
            {"deviceAppFirmware", "appFirmware"}, {"deviceMotorFirmware", "motorFirmware"},
            {"deviceInstallMode", "installMode"}, {"deviceFOVstart", "fovStart"}, {"deviceFOVend", "fovEnd"},
            {"deviceMainVoltage", "mainVoltage"}, {"deviceInputVoltage", "inputVoltage"},
            {"deviceBaseVoltage", "baseVoltage"}, {"deviceTemperature", "temperatureC"},
            {"deviceIMUquaternion", "imuQuaternion"}, {"deviceIMUtranslation", "imuTranslation"},
            {"deviceIMUcalibrationValid", "imuCalibrationValid"}};
    }

    _Airy::_Airy()
    {
        m_config = {{"deviceIP", "192.168.1.200"}, {"bindIP", "0.0.0.0"},
            {"msopPort", 6699}, {"difopPort", 7788}, {"imuPort", 6688}, {"receiveBufferBytes", 4194304},
            {"tOutSec", 3}, {"frameTimeoutMs", 250}, {"nMaxFramePoints", 200000},
            {"bReflectivity", true}, {"bIMUoutput", true}, {"bIMUpreview", false}, {"bUseSensorTime", true}, {"splitAngle", 0.0},
            {"vRangeD", {0.1, 60.0}}, {"vColorDefault", {1.0, 1.0, 1.0}}};
    }

    _Airy::~_Airy() { stop(); }

    bool _Airy::loadConfig()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        IF_F(!_ModuleBase::loadConfig());
        json patch = json::object(), errors;
        for (const auto &field : m_config.items())
        {
            if (m_pJ->contains(field.key()))
            {
                patch[field.key()] = m_pJ->at(field.key());
            }
        }
        if (!applyConfig(patch, false, errors)) { LOG_E(errors.dump()); return false; }
        m_framePoints.reserve(m_config.at("nMaxFramePoints").get<size_t>());
        return true;
    }

    bool _Airy::saveConfig(bool bExport)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        IF_F(!_ModuleBase::saveConfig(false));
        m_pJ->update(m_config);
        return !bExport || m_pJcfg->saveToFile();
    }

    bool _Airy::link(InstanceMgr *pM)
    {
        IF_F(!_ModuleBase::link(pM));

        string tfName;
        jKv(*m_pJ, "_TransformRigid", tfName);
        m_pTf = dynamic_cast<_TransformRigid *>(static_cast<_ModuleBase *>(pM->findModule(tfName)));
        IF_Le_F(!tfName.empty() && !m_pTf, "_TransformRigid not found: " + tfName);

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

    bool _Airy::check() { return m_pPCLout && _ModuleBase::check(); }

    bool _Airy::openSockets(const json &config, string &error)
    {
        const string ip = config.at("bindIP");
        const int buffer = config.at("receiveBufferBytes");
        const int msop = openUDP(ip, config.at("msopPort"), buffer, error);
        if (msop < 0) return false;
        const int difop = openUDP(ip, config.at("difopPort"), buffer, error);
        if (difop < 0) { close(msop); return false; }
        const int imuPort = config.at("imuPort");
        const int imu = imuPort ? openUDP(ip, imuPort, buffer, error) : -1;
        if (imuPort && imu < 0) { close(msop); close(difop); return false; }
        closeSockets();
        m_msopSocket = msop;
        m_difopSocket = difop;
        m_imuSocket = imu;
        ++m_socketGeneration;
        return true;
    }

    void _Airy::closeSockets()
    {
        if (m_msopSocket >= 0) close(m_msopSocket);
        if (m_difopSocket >= 0) close(m_difopSocket);
        if (m_imuSocket >= 0) close(m_imuSocket);
        m_msopSocket = m_difopSocket = m_imuSocket = -1;
        ++m_socketGeneration;
    }

    bool _Airy::start()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (!check()) return false;
        if (m_started) return true;
        if (!openSockets(m_config, m_lastError)) { LOG_E(m_lastError); return false; }
        resetSensor(true);
        m_started = true;
        m_lastError.clear();
        if (m_pT->startThread(getUpdate, this)) return true;
        m_started = false;
        closeSockets();
        m_lastError = "Could not start Airy receive thread";
        return false;
    }

    void _Airy::stop()
    {
        // Do not hold the state lock while joining a worker that needs it.
        if (m_pT) { m_pT->stop(); m_pT->join(); }
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        closeSockets();
        m_started = false;
        resetFrame();
        resetIMUpreview();
    }

    void _Airy::update()
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();
            if (!m_pT->bRun()) break;
            pollfd sockets[3]{};
            uint64_t generation;
            {
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
                sockets[0] = {m_msopSocket, POLLIN, 0};
                sockets[1] = {m_difopSocket, POLLIN, 0};
                sockets[2] = {m_imuSocket, POLLIN, 0};
                generation = m_socketGeneration;
            }
            const int ready = poll(sockets, 3, 10);
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            // A web configuration update can replace the sockets during poll.
            if (generation != m_socketGeneration) continue;
            if (ready < 0 && errno != EINTR) m_lastError = "UDP poll: " + string(strerror(errno));
            if (ready > 0)
                // Calibration and IMU must not wait behind the point backlog.
                for (int i : {1, 2, 0})
                {
                    if (sockets[i].revents & POLLIN) receive(sockets[i].fd, i);
                    if (sockets[i].revents & (POLLERR | POLLNVAL)) m_lastError = "Airy UDP receive socket error";
                }
            const uint64_t now = getTns();
            if (m_frameActive && m_lastMSOP && now - m_lastMSOP >= m_config.at("frameTimeoutMs").get<uint64_t>() * NSEC_MSEC)
                publishFrame(true);
            const uint64_t last = std::max({m_lastMSOP, m_lastDIFOP, m_lastIMU});
            if (last && now - last >= m_config.at("tOutSec").get<uint64_t>() * NSEC_SEC && !m_timedOut)
            {
                m_timedOut = true;
                resetFrame();
                if (m_config.at("deviceIP") == "0.0.0.0")
                { m_selectedIP.clear(); m_calibration = airy::Calibration{}; }
                m_lastIMUstamp = 0;
                resetIMUpreview();
                if (m_pIMUout) m_pIMUout->clear();
            }
        }
    }

    void _Airy::receive(int fd, int kind)
    {
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
                if (kind == 0)
                {
                    if (!m_calibration.valid && airy::validMsopHeader(bytes, static_cast<size_t>(length)))
                    { valid = true; ++m_uncalibratedPackets; m_lastMSOP = now; }
                    else
                    {
                        airy::Msop packet;
                        if ((valid = airy::decodeMsop(bytes, static_cast<size_t>(length), m_calibration, packet, &error))) handleMSOP(packet, now);
                    }
                }
                else if (kind == 1)
                {
                    airy::Difop packet;
                    if ((valid = airy::decodeDifop(bytes, static_cast<size_t>(length), packet, &error))) handleDIFOP(packet, now);
                }
                else
                {
                    airy::Imu packet;
                    if ((valid = airy::decodeImu(bytes, static_cast<size_t>(length), packet, &error))) handleIMU(packet, now);
                }
            }
            if (!valid) { ++m_invalidPackets; continue; }
            m_selectedIP = address;
            m_timedOut = false;
        }
    }

    void _Airy::resetFrame()
    {
        m_framePoints.clear();
        m_frameActive = false;
        m_framePartial = false;
        m_haveSequence = m_havePacket = m_haveAzimuth = false;
        m_frameStamp = m_lastPacketNs = 0;
    }

    void _Airy::resetSensor(bool bClearOutput)
    {
        resetFrame();
        m_lastMSOP = m_lastDIFOP = m_lastIMU = m_lastIMUstamp = 0;
        m_selectedIP.clear();
        m_observed = json::object();
        m_calibration = airy::Calibration{};
        m_latestIMU = airy::Imu{};
        resetIMUpreview();
        m_timedOut = false;
        if (bClearOutput && m_pIMUout)
        {
            m_pIMUout->clear();
        }
    }

    void _Airy::publishFrame(bool partial)
    {
        if (!m_frameActive) return;
        // Keep capture time consistent with point times. A sensor clock reset
        // remains visible to downstream SLAM instead of inventing a new epoch.
        const uint64_t stamp = std::max<uint64_t>(m_frameStamp, 1);
        if (m_pPCLout) m_pPCLout->set(m_framePoints, stamp);
        m_lastPublishedStamp = stamp;
        ++m_frames;
        if (partial || m_framePartial) ++m_partialFrames;
        m_framePoints.clear();
        m_frameActive = false;
        m_framePartial = false;
    }

    void _Airy::handleMSOP(const airy::Msop &packet, uint64_t receivedNs)
    {
        if (!m_calibration.valid) { ++m_uncalibratedPackets; return; }
        const bool sensorTime = m_config.at("bUseSensorTime");
        const uint64_t packetNs = sensorTime ? packet.timestampNs : receivedNs;
        uint32_t maxOffset = 0;
        for (const auto &point : packet.points) maxOffset = std::max(maxOffset, point.offsetNs);
        if (packetNs > std::numeric_limits<uint64_t>::max() - maxOffset)
        { ++m_invalidPackets; return; }
        ++m_pointPackets;
        if (m_lastMSOP && receivedNs > m_lastMSOP && receivedNs - m_lastMSOP >=
            m_config.at("tOutSec").get<uint64_t>() * NSEC_SEC)
        { publishFrame(true); resetFrame(); }
        m_lastMSOP = receivedNs;
        const bool clockReset = m_havePacket && m_lastPacketNs > packet.timestampNs && m_lastPacketNs - packet.timestampNs >= NSEC_SEC;
        if (clockReset)
        {
            publishFrame(true); resetFrame(); ++m_clockResets;
            m_lastIMUstamp = 0;
            resetIMUpreview();
            if (m_pIMUout) m_pIMUout->clear();
        }
        if (m_havePacket && packet.timestampNs < m_lastPacketNs) { ++m_outOfOrderPackets; return; }
        if (m_haveSequence && packet.sequenceValid)
        {
            // Airy's 32-bit wire field contains a counter modulo 65536.
            const uint32_t delta = (packet.sequence - m_lastSequence) & 0xffff;
            if (!delta || delta > 0x8000) { ++m_outOfOrderPackets; return; }
            if (delta > 1) { m_droppedPackets += delta - 1; m_framePartial = true; }
        }
        if (m_observed.contains("model") && (m_observed.at("model") != packet.model ||
            m_observed.at("msopReturnMode") != packet.returnMode))
        { publishFrame(true); resetFrame(); }
        m_observed["temperatureC"] = packet.temperature;
        m_observed["topTemperatureC"] = packet.topTemperature;
        m_observed["model"] = packet.model;
        m_observed["msopReturnMode"] = packet.returnMode;
        m_observed["dualReturn"] = packet.dual;
        m_lastPacketNs = packet.timestampNs;
        m_lastSequence = packet.sequence;
        m_haveSequence = packet.sequenceValid;
        m_havePacket = true;
        const float minimum = m_config.at("vRangeD")[0], maximum = m_config.at("vRangeD")[1];
        const size_t limit = m_config.at("nMaxFramePoints");
        const bool reflectivity = m_config.at("bReflectivity");
        const Vector3f color(m_config.at("vColorDefault")[0], m_config.at("vColorDefault")[1], m_config.at("vColorDefault")[2]);
        const uint16_t split = static_cast<uint16_t>(std::lround(m_config.at("splitAngle").get<double>() * 100));
        const bool croppedFOV = (unsigned(m_calibration.fovEnd) + 36000 - m_calibration.fovStart) % 36000 != 0;
        bool overflow = false;
        uint8_t previousBlock = 255;
        const Isometry3f pose = m_pTf ? m_pTf->getPoseCombinedf() : Isometry3f::Identity();
        for (const auto &sample : packet.points)
        {
            // Split only at block boundaries: channel calibration/interpolation
            // crosses zero at different points, and dual echoes share azimuths.
            if (sample.block != previousBlock)
            {
                previousBlock = sample.block;
                const uint16_t angle = static_cast<uint16_t>((unsigned(sample.blockAzimuth) + 36000 - split) % 36000);
                // A narrow configured FOV omits its blind sector, so the reset
                // at the next revolution can be smaller than half a turn.
                const bool boundary = m_haveAzimuth && angle < m_lastAzimuth &&
                    (croppedFOV || m_lastAzimuth - angle > 18000);
                if (boundary) publishFrame();
                if (!m_frameActive)
                {
                    m_frameActive = true;
                    m_frameStamp = std::max<uint64_t>(packetNs + sample.offsetNs, 1);
                    m_framePartial = !boundary;
                }
                m_lastAzimuth = angle;
                m_haveAzimuth = true;
            }
            const uint64_t stamp = std::max<uint64_t>(packetNs + sample.offsetNs, 1);
            m_frameStamp = std::min(m_frameStamp, stamp);
            if (sample.range <= 0 || sample.range < minimum || sample.range > maximum) continue;
            if (m_framePoints.size() >= limit) { overflow = true; m_framePartial = true; continue; }
            GEOMETRY_POINT point;
            point.m_vP = pose * Vector3f(sample.x, sample.y, sample.z);
            point.m_vC = reflectivity ? Vector3f::Constant(sample.intensity / 255.0f) : color;
            point.m_tStamp = stamp;
            m_framePoints.push_back(point);
            ++m_points;
        }
        if (overflow) ++m_frameOverflows;
    }

    void _Airy::handleDIFOP(const airy::Difop &packet, uint64_t receivedNs)
    {
        ++m_difopPackets;
        m_lastDIFOP = receivedNs;
        if (!(packet.calibration == m_calibration))
        {
            // Never combine points computed with two calibration/firing tables.
            resetFrame();
            m_calibration = packet.calibration;
        }
        m_observed.update({{"sensorIp", packet.sensorIp}, {"destinationIp", packet.destinationIp}, {"mac", packet.mac},
            {"msopPort", packet.msopPort}, {"difopPort", packet.difopPort}, {"rpm", packet.rpm}, {"motorSpeed", packet.motorSpeed},
            {"serial", packet.serial}, {"mainFirmware", packet.mainFirmware}, {"baseFirmware", packet.baseFirmware},
            {"appFirmware", packet.appFirmware}, {"motorFirmware", packet.motorFirmware},
            {"returnMode", packet.returnMode}, {"timeMode", packet.timeMode}, {"timeStatus", packet.timeStatus},
            {"timestampNs", std::to_string(packet.timestampNs)}, {"mainVoltage", packet.mainVoltage},
            {"inputVoltage", packet.inputVoltage}, {"baseVoltage", packet.baseVoltage}, {"temperatureC", packet.temperature},
            {"installMode", packet.calibration.installMode}, {"fovStart", packet.calibration.fovStart / 100.0},
            {"fovEnd", packet.calibration.fovEnd / 100.0}, {"imuQuaternion", packet.imuQuaternion},
            {"imuTranslation", packet.imuTranslation}, {"imuCalibrationValid", packet.imuCalibrationValid}});
    }

    void _Airy::handleIMU(const airy::Imu &packet, uint64_t receivedNs)
    {
        ++m_imuPackets;
        m_lastIMU = receivedNs;
        const uint64_t stamp = std::max<uint64_t>(m_config.at("bUseSensorTime").get<bool>() ? packet.timestampNs : receivedNs, 1);
        if (m_lastIMUstamp && stamp <= m_lastIMUstamp)
        {
            if (m_lastIMUstamp - stamp < NSEC_SEC) { ++m_imuOutOfOrder; return; }
            if (m_pIMUout) m_pIMUout->clear();
            resetIMUpreview();
            ++m_clockResets;
        }
        m_latestIMU = packet;
        m_lastIMUstamp = stamp;
        const Vector3f gyro(packet.angularVelocity[0], packet.angularVelocity[1], packet.angularVelocity[2]);
        const Vector3f acc(packet.acceleration[0], packet.acceleration[1], packet.acceleration[2]);
        if (m_bIMUpreview)
        {
            if (m_previewStamp && stamp - m_previewStamp > NSEC_SEC / 2) resetIMUpreview();
            if (!acc.allFinite() || !gyro.allFinite() || !std::isfinite(acc.squaredNorm()) || acc.squaredNorm() < 1e-12f)
                resetIMUpreview();
            else
            {
                if (!m_haveIMUleveling)
                {
                    // Airy's IMU can report gravity along -Z. Start from gravity
                    // instead of Mahony's identity (which cannot correct an exactly
                    // opposite gravity vector). This temporary coordinate rotation
                    // affects only fusion; compose it back into the displayed pose.
                    m_imuLeveling = Quaterniond::FromTwoVectors(acc.cast<double>(), Vector3d::UnitZ());
                    m_haveIMUleveling = true;
                }
                m_imuPreview.addAcc((m_imuLeveling * acc.cast<double>()).cast<float>(), stamp);
                m_imuPreview.addGyro((m_imuLeveling * gyro.cast<double>()).cast<float>(), stamp);
            }
            m_previewStamp = stamp;
        }
        if (!m_config.at("bIMUoutput").get<bool>()) return;
        // Raw IMU axes are intentionally preserved. DIFOP's extrinsic direction
        // and translation units must be established before an IMU SLAM preset.
        if (m_pIMUout)
        {
            m_pIMUout->addGyro(gyro, stamp);
            m_pIMUout->addAcc(acc, stamp);
        }
        ++m_imuSamples;
    }

    json _Airy::imuValues() const
    {
        const uint64_t now = getTns(), last = std::max({m_lastMSOP, m_lastDIFOP, m_lastIMU});
        const bool open = m_started && last && now - last < m_config.at("tOutSec").get<uint64_t>() * NSEC_SEC;
        const bool rawAvailable = open && m_lastIMU && now - m_lastIMU < NSEC_SEC;
        const bool available = m_bIMUpreview && rawAvailable;
        json values = {{"enabled", m_config.at("bIMUoutput")}, {"previewEnabled", m_bIMUpreview},
            {"available", available}, {"rawAvailable", rawAvailable}, {"deviceOpen", open},
            {"timestampNs", std::to_string(m_lastIMUstamp)}, {"sensorTimestampNs", std::to_string(m_latestIMU.timestampNs)},
            {"tGyro", std::to_string(m_lastIMUstamp)}, {"tAcc", std::to_string(m_lastIMUstamp)}, {"tFusion", "0"},
            {"gyro", m_latestIMU.angularVelocity}, {"acc", m_latestIMU.acceleration},
            {"quaternion", {1.0, 0.0, 0.0, 0.0}}, {"rpy", {0.0, 0.0, 0.0}}, {"orientationValid", false},
            {"gyroUnits", "rad/s"}, {"accUnits", "m/s²"}, {"axes", "raw sensor IMU frame"},
            {"outputRate", m_latestIMU.outputRate}, {"accelRange", m_latestIMU.accelRange},
            {"gyroRange", m_latestIMU.gyroRange}, {"temperatureRaw", m_latestIMU.temperatureRaw},
            {"fusion", m_bIMUpreview}, {"orientationFrame", "gravity-aligned preview; yaw is relative and may drift"}};
        if (!m_bIMUpreview) return values;
        const auto preview = m_imuPreview.snapshot();
        const Quaterniond orientation = (preview.quaternion * m_imuLeveling).normalized();
        const Vector3d rpy = orientation.toRotationMatrix().canonicalEulerAngles(2, 1, 0).reverse();
        values["tFusion"] = std::to_string(preview.tFusion);
        values["quaternion"] = {orientation.w(), orientation.x(), orientation.y(), orientation.z()};
        values["rpy"] = {rpy.x(), rpy.y(), rpy.z()};
        values["orientationValid"] = available && m_haveIMUleveling && preview.orientationValid;
        return values;
    }

    void _Airy::resetIMUpreview()
    {
        if (!m_bIMUpreview) return;
        m_imuPreview.reset();
        m_imuLeveling.setIdentity();
        m_previewStamp = 0;
        m_haveIMUleveling = false;
    }

    void _Airy::clear()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        resetFrame();
        if (m_pPCLout) m_pPCLout->set({}, ++m_lastPublishedStamp);
        if (m_pIMUout) m_pIMUout->clear();
        m_lastIMUstamp = 0;
        resetIMUpreview();
    }

    json _Airy::configValues() const
    {
        json values = m_config;
        for (const auto &field : deviceFields) values[field.first] = m_observed.value(field.second, json());
        return values;
    }

    json _Airy::controlSchema() const
    {
        json schema = json::object();
        schema["deviceIP"] = {{"type", "string"}, {"label", "LiDAR source IPv4"}, {"category", "Network"},
            {"description", "192.168.1.200 is the factory address; 0.0.0.0 accepts the first valid Airy sender. This is a host filter and does not change the sensor address."}};
        schema["bindIP"] = {{"type", "string"}, {"label", "Host bind IPv4"}, {"category", "Network"},
            {"description", "0.0.0.0 listens on all host interfaces. The host must own the LiDAR destination address (factory default 192.168.1.102)."}};
        for (const char *key : {"msopPort", "difopPort", "imuPort"})
            schema[key] = {{"type", "integer"}, {"label", key == string("msopPort") ? "Host MSOP UDP port" : key == string("difopPort") ? "Host DIFOP UDP port" : "Host IMU UDP port (0 disables)"},
                {"category", "Network"}, {"min", key == string("imuPort") ? 0 : 1}, {"max", 65535}, {"description", "Host listen port; does not change the sensor destination port."}};
        schema["receiveBufferBytes"] = {{"type", "integer"}, {"label", "UDP receive buffer (bytes)"}, {"category", "Network"}, {"min", 65536}, {"max", 67108864},
            {"description", "Requested socket buffer; the operating system may cap this value."}};
        schema["tOutSec"] = {{"type", "integer"}, {"label", "Disconnect timeout (s)"}, {"category", "Network"}, {"min", 1}, {"max", 120}};
        schema["frameTimeoutMs"] = {{"type", "integer"}, {"label", "Partial frame timeout (ms)"}, {"category", "Point cloud"}, {"min", 20}, {"max", 2000},
            {"description", "Publish the remaining partial frame after this interval without MSOP packets. Normal frames split at the configured azimuth once per revolution."}};
        schema["nMaxFramePoints"] = {{"type", "integer"}, {"label", "Maximum points per frame"}, {"category", "Point cloud"}, {"min", 96}, {"max", 1000000}};
        schema["bReflectivity"] = {{"type", "bool"}, {"label", "Reflectivity coloring"}, {"category", "Point cloud"}};
        schema["bIMUoutput"] = {{"type", "bool"}, {"label", "Publish raw IMU samples"}, {"category", "IMU"},
            {"description", "Publish accelerometer and gyroscope values in SI units in the raw IMU axes; no orientation fusion or LiDAR extrinsic transform is applied."}};
        schema["bIMUpreview"] = {{"type", "bool"}, {"label", "Enable IMU preview"}, {"category", "IMU"},
            {"description", "Run viewer-only orientation fusion. Disabled by default to save CPU; raw IMU publication is independent."}};
        schema["splitAngle"] = {{"type", "float"}, {"label", "Frame split azimuth (degrees)"}, {"category", "Point cloud"}, {"min", 0}, {"max", 359.99},
            {"description", "Start each scan at this raw motor azimuth; splitting occurs at block boundaries."}};
        schema["bUseSensorTime"] = {{"type", "bool"}, {"label", "Use sensor timestamps"}, {"category", "Point cloud"},
            {"description", "When disabled, timestamp packets with host monotonic receive time, retaining per-point offsets."}};
        schema["vRangeD"] = {{"type", "range"}, {"label", "Distance range (m)"}, {"category", "Point cloud"}, {"length", 2}, {"min", 0}, {"max", 81.915}};
        schema["vColorDefault"] = {{"type", "array"}, {"label", "Default RGB color"}, {"category", "Point cloud"}, {"length", 3}, {"min", 0}, {"max", 1}};
        for (const auto &field : deviceFields)
            schema[field.first] = {{"type", "string"}, {"label", field.second}, {"category", "Sensor (DIFOP)"}, {"readOnly", true},
                {"description", "Observed sensor value. Configure hardware settings through the Airy built-in web interface; DIFOP is output-only."}};
        return schema;
    }

    bool _Airy::applyConfig(const json &patch, bool live, json &errors)
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
                if (value != values.at(field.key())) errors[field.key()] = "Sensor parameter is read-only: use the Airy built-in web interface to configure hardware";
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
            }
            else if (type == "float") valid = value.is_number() && std::isfinite(value.get<double>()) && value.get<double>() >= spec.at("min").get<double>() && value.get<double>() <= spec.at("max").get<double>();
            else valid = value.is_number_integer() && value.get<double>() >= spec.at("min").get<double>() && value.get<double>() <= spec.at("max").get<double>();
            if (!valid) { errors[field.key()] = "Invalid type or value outside supported range"; continue; }
            next[field.key()] = value;
        }
        if (next.at("msopPort") == next.at("difopPort")) errors["difopPort"] = "MSOP and DIFOP host ports must differ";
        if (next.at("imuPort") != 0 && (next.at("imuPort") == next.at("msopPort") || next.at("imuPort") == next.at("difopPort")))
            errors["imuPort"] = "IMU port must differ from MSOP and DIFOP ports";
        if (!errors.empty()) return false;
        bool networkChanged = false;
        for (const char *key : {"bindIP", "msopPort", "difopPort", "imuPort", "receiveBufferBytes"}) networkChanged |= next.at(key) != m_config.at(key);
        if (live && m_started && networkChanged)
        {
            string error;
            if (!openSockets(next, error)) { errors["network"] = error; return false; }
        }
        if (next != m_config)
        {
            bool scanChanged = false;
            for (const auto &field : next.items())
                if (field.key() != "bIMUpreview" && field.value() != m_config.at(field.key())) scanChanged = true;
            if (scanChanged) resetFrame();
            if (networkChanged || next.at("deviceIP") != m_config.at("deviceIP"))
            {
                resetSensor(live);
                m_lastError.clear();
            }
            if (live && m_pIMUout && (m_config.at("bUseSensorTime") != next.at("bUseSensorTime") ||
                m_config.at("bIMUoutput") != next.at("bIMUoutput")))
            {
                m_pIMUout->clear();
            }
            if (m_config.at("bUseSensorTime") != next.at("bUseSensorTime"))
            { m_lastIMUstamp = 0; resetIMUpreview(); }
            m_config = next;
        }
        const bool previewEnabled = next.at("bIMUpreview");
        if (m_bIMUpreview != previewEnabled)
        {
            m_bIMUpreview = previewEnabled;
            if (m_bIMUpreview) resetIMUpreview();
            else { m_previewStamp = 0; m_haveIMUleveling = false; }
        }
        return true;
    }

    json _Airy::statusValues() const
    {
        const uint64_t now = getTns(), last = std::max({m_lastMSOP, m_lastDIFOP, m_lastIMU});
        const uint64_t timeout = m_config.at("tOutSec").get<uint64_t>() * NSEC_SEC;
        const bool connected = m_started && last && now - last < timeout;
        const bool streaming = connected && m_calibration.valid && m_lastMSOP && now - m_lastMSOP < timeout;
        const string state = !m_lastError.empty() ? "error" : !m_started ? "stopped" : streaming ? "streaming" :
            connected ? (m_lastMSOP && !m_calibration.valid ? "waiting for calibration" : "device information only") : last ? "disconnected" : "waiting";
        return {{"state", state}, {"deviceOpen", connected}, {"sourceIP", m_selectedIP},
            {"pointPackets", m_pointPackets}, {"difopPackets", m_difopPackets}, {"points", m_points},
            {"calibrationReady", m_calibration.valid}, {"uncalibratedPackets", m_uncalibratedPackets},
            {"imuPackets", m_imuPackets}, {"imuSamples", m_imuSamples}, {"imuOutOfOrder", m_imuOutOfOrder}, {"imu", imuValues()},
            {"lastIMUageMs", m_lastIMU ? json((now - m_lastIMU) / NSEC_MSEC) : json()},
            {"frames", m_frames}, {"partialFrames", m_partialFrames}, {"droppedPackets", m_droppedPackets},
            {"invalidPackets", m_invalidPackets}, {"outOfOrderPackets", m_outOfOrderPackets},
            {"frameOverflows", m_frameOverflows}, {"clockResets", m_clockResets},
            {"lastPacketAgeMs", last ? json((now - last) / NSEC_MSEC) : json()},
            {"lastMSOPageMs", m_lastMSOP ? json((now - m_lastMSOP) / NSEC_MSEC) : json()},
            {"lastDIFOPageMs", m_lastDIFOP ? json((now - m_lastDIFOP) / NSEC_MSEC) : json()},
            {"deviceControlSupported", false}, {"lastError", m_lastError}, {"observed", m_observed}};
    }

    void _Airy::console(void *pConsole)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        _ModuleBase::console(pConsole);
        if (pConsole) static_cast<_Console *>(pConsole)->addMsg("Airy " + statusValues().at("state").get<string>() +
            " " + m_selectedIP + " frames=" + std::to_string(m_frames) + " " + m_lastError);
    }

    void _Airy::console(const json &request, void *pJSONbase)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto *endpoint = static_cast<_JSONbase *>(pJSONbase);
        if (!endpoint || !request.is_object() || !request.contains("cmd") || !request.at("cmd").is_string()) return;
        const string command = request.at("cmd");
        const bool set = command == "setConfig" || command == "config.set";
        const bool save = command == "saveConfig" || command == "config.save";
        if (!set && !save && command != "getConfig" && command != "config.get" && command != "config.schema" && command != "status" && command != "getIMU") return;
        json reply = {{"cmd", command}, {"module", getName()}, {"bSuccess", true}};
        if (request.contains("requestId")) reply["requestId"] = request.at("requestId");
        try
        {
            if (command == "getIMU")
            {
                reply.update(imuValues());
                endpoint->sendJson(reply);
                return;
            }
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
