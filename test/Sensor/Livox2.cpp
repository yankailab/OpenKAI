#ifdef OPENKAI_LIVOX2_TEST
#include "../../src/Sensor/LiDAR/Livox/_Livox2.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <poll.h>
#include <thread>

namespace kai
{
    struct Livox2TestAccess
    {
        static void outputs(_Livox2 &driver, PCLframe &cloud, IMUstream &imu)
        { driver.m_pPCLout = &cloud; driver.m_pIMUout = &imu; }
        static bool command(_Livox2 &driver, const vector<uint8_t> &bytes, LIVOX2_CMD &result)
        { return driver.recvLivoxCmd(bytes.data(), bytes.size(), &result); }
        static bool data(_Livox2 &driver, const vector<uint8_t> &bytes, LIVOX2_DATA &result)
        { return driver.recvLivoxData(bytes.data(), bytes.size(), &result); }
        static void points(_Livox2 &driver, const LIVOX2_DATA &packet) { driver.handlePointCloudData(packet); }
        static void imu(_Livox2 &driver, const LIVOX2_DATA &packet) { driver.handleIMUdata(packet); }
        static json snapshot(_Livox2 &driver) { return driver.imuValues(); }
        // Inspect dormant state directly to detect accidental updates or resets while disabled.
        static IMUpreview::Snapshot previewState(_Livox2 &driver) { return driver.m_imuPreview.snapshot(); }
        static json schema(_Livox2 &driver) { return driver.controlSchema(); }
        static bool config(_Livox2 &driver, const json &patch, json &errors) { return driver.applyConfig(patch, true, errors); }
        static json values(_Livox2 &driver) { return driver.configValues(); }
        static void queue(_Livox2 &driver, uint16_t key, uint8_t value) { driver.setLvxByte(key, value); }
        static size_t pending(_Livox2 &driver) { return driver.m_pending.size(); }
        static uint32_t sequence(_Livox2 &driver) { return driver.m_sequence; }
        static bool failed(_Livox2 &driver) { return driver.m_configFailed; }
        static void connected(_Livox2 &driver) { driver.m_lvxState = lvxState_work; }
        static vector<LIVOX2_CMD> pendingCommands(_Livox2 &driver)
        {
            vector<LIVOX2_CMD> result;
            for (const auto &pending : driver.m_pending) result.push_back(pending.second.command);
            return result;
        }
        static void ack(_Livox2 &driver, const LIVOX2_CMD &reply) { driver.handleCtrlCmdAck(reply); }
        static void info(_Livox2 &driver, const vector<uint8_t> &tlv, uint16_t count) { driver.handleInfo(tlv.data(), tlv.size(), count); }
        static json observed(_Livox2 &driver) { return driver.m_observed; }
        static bool offlineConfig(_Livox2 &driver, const json &patch, json &errors)
        { return driver.applyConfig(patch, false, errors); }
        static bool open(_Livox2 &driver)
        { string error; return driver.openSockets(error); }
        static void commandPort(_Livox2 &driver, uint16_t port) { driver.m_lvxCmdPort = port; }
        static void service(_Livox2 &driver, uint64_t now) { driver.service(now); }
        static void timers(_Livox2 &driver, uint64_t now) { driver.m_lastRx = driver.m_lastQuery = now; }
        static uint64_t sent(_Livox2 &driver) { return driver.m_pending.begin()->second.sent; }
        static void receiveQuery(_Livox2 &driver) { driver.receive(_Livox2::Query); }
        static json transport(_Livox2 &driver)
        {
            std::lock_guard<std::recursive_mutex> lock(driver.m_mutex);
            bool socketsClosed = true;
            for (int fd : driver.m_sockets) socketsClosed &= fd < 0;
            return {{"state", driver.m_lvxState}, {"closed", socketsClosed},
                {"pointPackets", driver.m_pointPackets}, {"imuSamples", driver.m_imuSamples},
                {"invalidPackets", driver.m_invalidPackets}, {"outOfOrderPackets", driver.m_outOfOrderPackets},
                {"commandRetries", driver.m_commandRetries}, {"commandErrors", driver.m_commandErrors},
                {"pending", driver.m_pending.size()}, {"framePoints", driver.m_vFramePoints.size()},
                {"observed", driver.m_observed}};
        }
    };
}
namespace
{
    using namespace kai;
    using Access = Livox2TestAccess;

    struct Datagram
    {
        vector<uint8_t> bytes;
        sockaddr_in source{};
    };

    class UdpEndpoint
    {
    public:
        explicit UdpEndpoint(const char *ip = "127.0.0.1", uint16_t port = 0)
        {
            m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
            assert(m_fd >= 0);
            m_address.sin_family = AF_INET;
            m_address.sin_port = htons(port);
            assert(inet_pton(AF_INET, ip, &m_address.sin_addr) == 1);
            assert(::bind(m_fd, reinterpret_cast<sockaddr *>(&m_address), sizeof(m_address)) == 0);
            socklen_t size = sizeof(m_address);
            assert(::getsockname(m_fd, reinterpret_cast<sockaddr *>(&m_address), &size) == 0);
        }
        ~UdpEndpoint() { close(); }
        UdpEndpoint(const UdpEndpoint &) = delete;
        UdpEndpoint &operator=(const UdpEndpoint &) = delete;
        uint16_t port() const { return ntohs(m_address.sin_port); }
        void close() { if (m_fd >= 0) ::close(m_fd); m_fd = -1; }
        void send(const vector<uint8_t> &bytes, const sockaddr_in &destination)
        {
            assert(::sendto(m_fd, bytes.data(), bytes.size(), 0,
                reinterpret_cast<const sockaddr *>(&destination), sizeof(destination)) == ssize_t(bytes.size()));
        }
        void send(const vector<uint8_t> &bytes, uint16_t hostPort)
        {
            sockaddr_in destination{};
            destination.sin_family = AF_INET; destination.sin_port = htons(hostPort);
            destination.sin_addr.s_addr = inet_addr("127.0.0.1");
            send(bytes, destination);
        }
        void broadcast(const vector<uint8_t> &bytes, uint16_t hostPort)
        {
            const int enabled = 1;
            assert(::setsockopt(m_fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)) == 0);
            sockaddr_in destination{};
            destination.sin_family = AF_INET; destination.sin_port = htons(hostPort);
            destination.sin_addr.s_addr = inet_addr("127.255.255.255");
            send(bytes, destination);
        }
        Datagram receive()
        {
            pollfd descriptor{m_fd, POLLIN, 0};
            assert(::poll(&descriptor, 1, 2000) == 1 && (descriptor.revents & POLLIN));
            Datagram datagram; datagram.bytes.resize(65536);
            socklen_t size = sizeof(datagram.source);
            const ssize_t received = ::recvfrom(m_fd, datagram.bytes.data(), datagram.bytes.size(), 0,
                reinterpret_cast<sockaddr *>(&datagram.source), &size);
            assert(received >= 0);
            datagram.bytes.resize(size_t(received)); return datagram;
        }
        bool empty() const
        {
            pollfd descriptor{m_fd, POLLIN, 0};
            return ::poll(&descriptor, 1, 0) == 0;
        }
    private:
        int m_fd = -1;
        sockaddr_in m_address{};
    };

    struct DriverFixture
    {
        JsonCfg config;
        PCLframe cloud;
        IMUstream imu;
        _Livox2 driver;
        uint16_t commandPort, statePort, pointPort, imuPort;

        explicit DriverFixture(uint16_t queryPort, const char *hostIP = "0.0.0.0")
        {
            // Reserve all ports together so the kernel cannot choose one twice.
            UdpEndpoint command, state, point, inertial;
            commandPort = command.port(); statePort = state.port();
            pointPort = point.port(); imuPort = inertial.port();
            config.setJson({{"name", "livox-local-test"}, {"class", "_Livox2"},
                {"lvxIP", "127.0.0.2"}, {"lvxHostIP", hostIP}, {"lvxPortQuery", queryPort},
                {"lvxHostPortCmd", commandPort}, {"lvxHostPortState", statePort},
                {"lvxHostPortPCL", pointPort}, {"lvxHostPortIMU", imuPort}});
            driver.BASE::setConfig(&config, config.getJson());
            assert(driver.loadConfig());
            Access::outputs(driver, cloud, imu);
        }
    };

    template <class Predicate> void await(Predicate predicate)
    {
        const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (!predicate() && std::chrono::steady_clock::now() < end)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        assert(predicate());
    }

    vector<uint8_t> wire(LIVOX2_CMD command)
    {
        command.calcCRC();
        const auto *bytes = reinterpret_cast<const uint8_t *>(&command);
        return vector<uint8_t>(bytes, bytes + command.length);
    }

    bool hostCommand(const vector<uint8_t> &bytes, LIVOX2_CMD &command)
    {
        if (bytes.size() < 24 || bytes.size() > 1368 || bytes[0] != 0xaa || bytes[1] != 0 ||
            livox2::u16(bytes.data() + 2) != bytes.size() || bytes[10] != LVX2_CMD_REQ || bytes[11] != 0)
            return false;
        if (CRC::Calculate(bytes.data(), 18, CRC::CRC_16_CCITTFALSE()) != livox2::u16(bytes.data() + 18) ||
            (bytes.size() == 24 ? 0 : CRC::Calculate(bytes.data() + 24, bytes.size() - 24, CRC::CRC_32())) !=
                livox2::u32(bytes.data() + 20)) return false;
        memset(&command, 0, sizeof(command));
        memcpy(&command, bytes.data(), bytes.size()); return true;
    }

    void put(vector<uint8_t> &data, size_t offset, uint64_t value, size_t size)
    { for (size_t i = 0; i < size; ++i) data[offset + i] = uint8_t(value >> (8 * i)); }

    vector<uint8_t> packet(uint8_t type, uint16_t count, uint64_t stamp, bool withCRC = true)
    {
        vector<uint8_t> bytes(36 + livox2::pointSize(type) * count);
        put(bytes, 1, bytes.size(), 2); put(bytes, 3, 100, 2); put(bytes, 5, count, 2);
        bytes[10] = type; put(bytes, 28, stamp, 8);
        if (withCRC) put(bytes, 24, CRC::Calculate(bytes.data() + 28, bytes.size() - 28, CRC::CRC_32()), 4);
        return bytes;
    }

    void testProtocol()
    {
        _Livox2 driver; LIVOX2_DATA data; LIVOX2_CMD command;
        auto bytes = packet(1, 96, 1000000000);
        assert(Livox2TestAccess::data(driver, bytes, data));
        for (size_t size = 0; size < bytes.size(); ++size)
            assert(!livox2::validData(bytes.data(), size));
        auto corrupt = bytes; corrupt.back() ^= 1; assert(!livox2::validData(corrupt.data(), corrupt.size()));
        put(corrupt, 24, 0, 4); assert(livox2::validData(corrupt.data(), corrupt.size()));
        corrupt[10] = 4; assert(!livox2::validData(corrupt.data(), corrupt.size()));
        corrupt = bytes; put(corrupt, 5, 65535, 2); assert(!livox2::validData(corrupt.data(), corrupt.size()));
        corrupt = bytes; put(corrupt, 28, UINT64_MAX, 8); assert(!livox2::validData(corrupt.data(), corrupt.size(), false));
        auto imu = packet(0, 1, 1000000000);
        assert(livox2::validData(imu.data(), imu.size()));
        put(imu, 24, 0, 4); assert(!livox2::validData(imu.data(), imu.size()));
        assert(livox2::pointTime(100, 4750, 96, 95) == 475100);
        assert(livox2::pointTime(100, 4750, 96, 1) == 5100);
        assert(livox2::pointTime(100, 4750, 1, 0) == 100);

        command.init(LVX2_CMD_GET, LVX2_CMD_ACK, 42); command.sender_type = 1;
        command.addData(uint8_t(0)); command.addData(uint16_t(0)); command.calcCRC();
        bytes.assign(reinterpret_cast<uint8_t *>(&command), reinterpret_cast<uint8_t *>(&command) + command.length);
        LIVOX2_CMD received;
        assert(Livox2TestAccess::command(driver, bytes, received)); assert(received.seq_num == 42);
        for (size_t size = 0; size < bytes.size(); ++size) assert(!livox2::validCommand(bytes.data(), size));
        bytes[12] ^= 1; assert(!Livox2TestAccess::command(driver, bytes, received));
        command.init(LVX2_CMD_DISCOVER, LVX2_CMD_REQ, 1);
        for (const char c : command.rsvd) assert(c == 0);
        for (int i = 0; i < LVX2_N_DATA; ++i) command.addData(uint8_t(0));
        bool bounded = false; try { command.addData(uint8_t(0)); } catch (const std::length_error &) { bounded = true; }
        assert(bounded);
    }

    LIVOX2_DATA points(_Livox2 &driver, uint8_t type, uint64_t stamp)
    {
        auto bytes = packet(type, 3, stamp, false);
        if (type == 1)
        {
            LivoxLidarCartesianHighRawPoint raw[3] = {{1000, 0, 0, 255, 0}, {0, 0, 0, 60, 0}, {0, 2000, 0, 128, 0}};
            memcpy(bytes.data() + 36, raw, sizeof(raw));
        }
        else if (type == 2)
        {
            LivoxLidarCartesianLowRawPoint raw[3] = {{100, 0, 0, 255, 0}, {0, 0, 0, 60, 0}, {0, 200, 0, 128, 0}};
            memcpy(bytes.data() + 36, raw, sizeof(raw));
        }
        else
        {
            LivoxLidarSpherPoint raw[3] = {{1000, 9000, 0, 255, 0}, {0, 0, 0, 60, 0}, {2000, 9000, 9000, 128, 0}};
            memcpy(bytes.data() + 36, raw, sizeof(raw));
        }
        LIVOX2_DATA result;
        assert(Livox2TestAccess::data(driver, bytes, result)); return result;
    }

    void testCloud()
    {
        for (uint8_t type = 1; type <= 3; ++type)
        {
            PCLframe cloud; IMUstream imu; _Livox2 driver;
            Livox2TestAccess::outputs(driver, cloud, imu);
            const uint64_t start = 10000000000ULL;
            Livox2TestAccess::points(driver, points(driver, type, start));
            Livox2TestAccess::points(driver, points(driver, type, start)); // duplicate rejected
            Livox2TestAccess::points(driver, points(driver, type, start - 100)); // reordering rejected
            Livox2TestAccess::points(driver, points(driver, type, start + 100000000));
            vector<GEOMETRY_POINT> actual;
            assert(cloud.get(actual) == start); assert(actual.size() == 2);
            assert((actual[0].m_vP - Vector3f(1, 0, 0)).norm() < 1e-5f);
            assert((actual[1].m_vP - Vector3f(0, 2, 0)).norm() < 1e-5f);
            assert(actual[0].m_tStamp == start); assert(actual[1].m_tStamp == start + 10000);
            assert(actual[0].m_vC.x() == 1.0f);
            json errors;
            assert(Livox2TestAccess::config(driver, {{"vRangeD", {100, 101}}}, errors));
            Livox2TestAccess::points(driver, points(driver, type, start + 200000000));
            Livox2TestAccess::points(driver, points(driver, type, start + 300000000));
            cloud.get(actual); assert(actual.empty()); // filtering every return clears the viewer
            driver.clear(); cloud.get(actual); assert(actual.empty());
        }
    }

    vector<uint8_t> imuBytes(uint64_t stamp)
    {
        auto bytes = packet(0, 1, stamp);
        LivoxLidarImuRawPoint raw{0.1f, -0.2f, 0.3f, 0, 0, 1};
        memcpy(bytes.data() + 36, &raw, sizeof(raw));
        put(bytes, 24, CRC::Calculate(bytes.data() + 28, bytes.size() - 28, CRC::CRC_32()), 4);
        return bytes;
    }

    LIVOX2_DATA imuPacket(_Livox2 &driver, uint64_t stamp)
    {
        const auto bytes = imuBytes(stamp);
        LIVOX2_DATA data; assert(Livox2TestAccess::data(driver, bytes, data)); return data;
    }

    void testIMU()
    {
        PCLframe cloud; IMUstream imu; _Livox2 driver;
        Livox2TestAccess::outputs(driver, cloud, imu);
        const uint64_t stamp = 10000000000ULL;
        auto data = imuPacket(driver, stamp);
        Livox2TestAccess::imu(driver, data);
        Livox2TestAccess::imu(driver, data); // duplicate rejected
        Livox2TestAccess::imu(driver, points(driver, 1, stamp + 1)); // wrong stream type
        deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc); assert(gyro.size() == 1 && acc.size() == 1);
        assert(gyro[0].m_t == stamp && acc[0].m_t == stamp);
        assert(std::abs(acc[0].m_v.z() - 9.80665f) < 1e-5f);
        assert(std::abs(gyro[0].m_v.y() + 0.2f) < 1e-5f);
        for (int i = 0; i < 10; ++i) Livox2TestAccess::snapshot(driver);
        imu.get(gyro, acc); assert(gyro.size() == 1); // preview does not consume SLAM input
        Livox2TestAccess::imu(driver, imuPacket(driver, stamp + 5000000));
        assert(Livox2TestAccess::snapshot(driver).at("orientationValid") == false); // disconnected preview freshness
        Livox2TestAccess::imu(driver, imuPacket(driver, 1000000000)); // sensor clock restart clears old history
        imu.get(gyro, acc); assert(gyro.size() == 1 && gyro[0].m_t == 1000000000);
        json errors;
        assert(Livox2TestAccess::config(driver, {{"lvxIMUdataEn", 0}}, errors));
        Livox2TestAccess::imu(driver, imuPacket(driver, 1005000000));
        imu.get(gyro, acc); assert(gyro.empty() && acc.empty());
    }

    void testIMUPreview()
    {
        PCLframe cloud; IMUstream imu; _Livox2 driver;
        Access::outputs(driver, cloud, imu); Access::connected(driver);
        assert(Access::values(driver).at("bIMUpreview") == false);
        const auto spec = Access::schema(driver).at("bIMUpreview");
        assert(spec.at("type") == "bool" && spec.at("label") == "Enable IMU preview");
        const uint64_t stamp = 10000000000ULL;
        Access::imu(driver, imuPacket(driver, stamp));
        Access::imu(driver, imuPacket(driver, stamp + 5000000));
        Access::points(driver, points(driver, 1, stamp));
        assert(Access::transport(driver).at("framePoints") == 2);
        deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc); assert(gyro.size() == 2 && acc.size() == 2);
        auto state = Access::snapshot(driver);
        assert(state.at("enabled") == true && state.at("previewEnabled") == false);
        assert(state.at("available") == false && state.at("orientationValid") == false);
        assert(state.at("tGyro") == "0" && Access::previewState(driver).tGyro == 0);

        json errors;
        const auto before = Access::values(driver);
        assert(!Access::config(driver, {{"bIMUpreview", 1}, {"frameDurationMs", 25}}, errors));
        assert(errors.contains("bIMUpreview") && Access::values(driver) == before);
        assert(!Access::config(driver, {{"bIMUstab", true}, {"frameDurationMs", 25}}, errors));
        assert(errors.at("bIMUstab").get<string>().find("bIMUpreview=true") != string::npos);
        assert(Access::values(driver) == before);

        assert(Access::config(driver, {{"bIMUpreview", true}}, errors));
        assert(Access::transport(driver).at("framePoints") == 2);
        state = Access::snapshot(driver);
        assert(state.at("previewEnabled") == true && state.at("available") == false);
        imu.get(gyro, acc); assert(gyro.size() == 2 && acc.size() == 2);
        Access::imu(driver, imuPacket(driver, stamp + 5000000)); // toggling preserves raw deduplication
        imu.get(gyro, acc); assert(gyro.size() == 2);
        Access::imu(driver, imuPacket(driver, stamp + 10000000));
        state = Access::snapshot(driver);
        assert(state.at("available") == true && state.at("orientationValid") == false);
        Access::imu(driver, imuPacket(driver, stamp + 15000000));
        state = Access::snapshot(driver);
        assert(state.at("orientationValid") == true);
        assert(state.at("tGyro") == std::to_string(stamp + 15000000));
        for (int i = 0; i < 10; ++i) Access::snapshot(driver);
        imu.get(gyro, acc); assert(gyro.size() == 4 && acc.size() == 4);

        const auto active = Access::previewState(driver);
        assert(Access::config(driver, {{"bIMUpreview", false}}, errors));
        assert(Access::transport(driver).at("framePoints") == 2);
        Access::imu(driver, imuPacket(driver, stamp + 20000000));
        state = Access::snapshot(driver);
        assert(state.at("previewEnabled") == false && state.at("available") == false);
        assert(state.at("orientationValid") == false && state.at("tGyro") == "0");
        imu.get(gyro, acc); assert(gyro.size() == 5 && acc.size() == 5);
        assert(gyro.back().m_t == stamp + 20000000 && std::abs(acc.back().m_v.z() - 9.80665f) < 1e-5f);
        assert(Access::previewState(driver).tGyro == active.tGyro);
        assert(Access::config(driver, {{"lvxIMUdataEn", 0}}, errors));
        assert(Access::config(driver, {{"lvxIMUdataEn", 1}}, errors));
        Access::imu(driver, imuPacket(driver, stamp + 25000000));
        Access::imu(driver, imuPacket(driver, 1000000000)); // raw clock reset must not reset dormant preview
        imu.get(gyro, acc); assert(gyro.size() == 1 && gyro.front().m_t == 1000000000);
        driver.stop();
        const auto dormant = Access::previewState(driver);
        assert(dormant.tGyro == active.tGyro && dormant.tFusion == active.tFusion);
        assert(dormant.orientationValid && dormant.quaternion.coeffs() == active.quaternion.coeffs());

        assert(Access::config(driver, {{"bIMUpreview", true}}, errors));
        state = Access::snapshot(driver);
        assert(state.at("tGyro") == "0" && state.at("orientationValid") == false);
        assert(Access::previewState(driver).tFusion == 0);
        imu.get(gyro, acc); assert(gyro.size() == 1 && acc.size() == 1);
        Access::connected(driver);
        Access::imu(driver, imuPacket(driver, 1005000000));
        assert(Access::snapshot(driver).at("orientationValid") == false);
        Access::imu(driver, imuPacket(driver, 1010000000));
        assert(Access::snapshot(driver).at("orientationValid") == true);

        assert(Access::config(driver, {{"bIMUstab", true}}, errors));
        const auto stabilized = Access::values(driver);
        assert(!Access::config(driver, {{"bIMUpreview", false}, {"frameDurationMs", 25}}, errors));
        assert(errors.contains("bIMUstab") && Access::values(driver) == stabilized);
        assert(Access::config(driver, {{"bIMUpreview", false}, {"bIMUstab", false}}, errors));
    }

    void testIMUPreviewPersistence()
    {
        JsonCfg config;
        config.setJson({{"name", "livox-preview-test"}, {"class", "_Livox2"}, {"lvxIP", "127.0.0.2"}});
        _Livox2 driver;
        driver.BASE::setConfig(&config, config.getJson());
        assert(driver.loadConfig() && driver.saveConfig(false));
        assert(config.getJson()->at("bIMUpreview") == false);
        json errors;
        assert(Access::config(driver, {{"bIMUpreview", true}}, errors));
        assert(driver.saveConfig(false) && config.getJson()->at("bIMUpreview") == true);
        _Livox2 restored;
        restored.BASE::setConfig(&config, config.getJson());
        assert(restored.loadConfig() && Access::values(restored).at("bIMUpreview") == true);
        assert(Access::snapshot(restored).at("available") == false);
    }

    void testConfigurationAndACK()
    {
        UdpEndpoint sink("127.0.0.2");
        DriverFixture fixture(sink.port());
        _Livox2 &driver = fixture.driver;
        assert(Access::open(driver));
        Access::commandPort(driver, sink.port());
        const json before = Livox2TestAccess::values(driver); json errors;
        assert(!Livox2TestAccess::config(driver, {{"lvxPCLdataType", 99}, {"frameDurationMs", 25}}, errors));
        assert(Livox2TestAccess::values(driver) == before); // validation is transactional
        assert(!Livox2TestAccess::config(driver, {{"lvxHostIP", "1.2.3.4"}}, errors));
        assert(!Livox2TestAccess::config(driver, {{"vRangeD", {10, 1}}}, errors));
        assert(!Livox2TestAccess::config(driver, {{"vColorDefault", {0, "bad", 1}}}, errors));
        assert(!Livox2TestAccess::config(driver, {{"lvxPatternMode", 1}}, errors));
        assert(Livox2TestAccess::config(driver, {{"lvxPCLdataType", 2}, {"frameDurationMs", 25}}, errors));
        Livox2TestAccess::connected(driver);
        Livox2TestAccess::queue(driver, kKeyDetectMode, 1); const uint32_t old = Livox2TestAccess::sequence(driver);
        Livox2TestAccess::queue(driver, kKeyDetectMode, 0); assert(Livox2TestAccess::pending(driver) == 1);
        const uint32_t latest = Livox2TestAccess::sequence(driver); assert(latest != old);
        LIVOX2_CMD ack; ack.init(LVX2_CMD_SET, LVX2_CMD_ACK, old); ack.addData(uint8_t(0)); ack.addData(uint16_t(0));
        Livox2TestAccess::ack(driver, ack); assert(Livox2TestAccess::pending(driver) == 1); // stale ACK ignored
        ack.seq_num = latest; ack.data[0] = 0x20;
        Livox2TestAccess::ack(driver, ack); assert(Livox2TestAccess::failed(driver)); assert(Livox2TestAccess::pending(driver) == 0);
        assert(Livox2TestAccess::config(driver, {{"lvxDetectMode", 0}}, errors)); // retry identical desired value
        assert(Livox2TestAccess::pending(driver) == 6); // full config must be acknowledged
        for (const auto &pending : Livox2TestAccess::pendingCommands(driver))
        {
            ack.seq_num = pending.seq_num;
            ack.data[0] = livox2::u16(pending.data + 4) == kKeyDetectMode ? 0x20 : 0;
            Livox2TestAccess::ack(driver, ack);
        }
        assert(Livox2TestAccess::failed(driver)); assert(Livox2TestAccess::pending(driver) == 0);
        assert(Livox2TestAccess::config(driver, {{"lvxIMUdataEn", 0}}, errors)); // unrelated edit retries failed detect too
        const auto recovery = Livox2TestAccess::pendingCommands(driver);
        assert(recovery.size() == 6);
        bool retriesFailedKey = false;
        for (const auto &pending : recovery)
        {
            retriesFailedKey |= livox2::u16(pending.data + 4) == kKeyDetectMode;
            ack.seq_num = pending.seq_num; ack.data[0] = 0;
            Livox2TestAccess::ack(driver, ack);
        }
        assert(retriesFailedKey && !Livox2TestAccess::failed(driver) && Livox2TestAccess::pending(driver) == 0);
        Livox2TestAccess::info(driver, {0, 0, 1, 0, 3, 1, 0, 255, 255}, 2);
        assert(Livox2TestAccess::observed(driver).empty()); // malformed TLV cannot partially update status
        Livox2TestAccess::info(driver, {0, 0, 1, 0, 3}, 1);
        assert(Livox2TestAccess::observed(driver).at("lvxPCLdataType") == 3);
    }

    LIVOX2_CMD discoveryReply(uint32_t sequence, uint16_t commandPort, const char *ip = "127.0.0.2")
    {
        LIVOX2_CMD reply; reply.init(LVX2_CMD_DISCOVER, LVX2_CMD_ACK, sequence); reply.sender_type = 1;
        vector<uint8_t> payload(24);
        payload[1] = 9;
        memcpy(payload.data() + 2, "LOCALHOST-MID360", 15);
        assert(inet_pton(AF_INET, ip, payload.data() + 18) == 1);
        put(payload, 22, commandPort, 2);
        reply.addData(payload.data(), payload.size()); return reply;
    }

    void testOwnedSockets()
    {
        UdpEndpoint query("127.0.0.2"), commands("127.0.0.2"), sensor("127.0.0.2"), other("127.0.0.3");
        DriverFixture fixture(query.port());
        _Livox2 &driver = fixture.driver;
        assert(driver.start());
        const auto discovery = query.receive();
        LIVOX2_CMD request;
        assert(hostCommand(discovery.bytes, request));
        assert(request.cmd_id == LVX2_CMD_DISCOVER && request.cmd_type == LVX2_CMD_REQ);
        assert(discovery.source.sin_addr.s_addr == inet_addr("127.0.0.1"));

        // Both the UDP sender and the advertised address must identify this sensor.
        other.send(wire(discoveryReply(request.seq_num, commands.port())), discovery.source);
        query.send(wire(discoveryReply(request.seq_num, commands.port(), "127.0.0.3")), discovery.source);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        assert(Access::transport(driver).at("state") == lvxState_deviceQuery);
        // Mid-360 discovery ACKs arrive by broadcast, requiring a wildcard query bind.
        query.broadcast(wire(discoveryReply(request.seq_num, commands.port())), ntohs(discovery.source.sin_port));

        bool receivedHostConfig = false;
        for (int count = 0; count < 6; ++count)
        {
            const auto outgoing = commands.receive();
            LIVOX2_CMD command;
            assert(hostCommand(outgoing.bytes, command));
            assert(command.cmd_id == LVX2_CMD_SET && command.cmd_type == LVX2_CMD_REQ);
            assert(ntohs(outgoing.source.sin_port) == fixture.commandPort);
            if (livox2::u16(command.data) == 3)
            {
                const uint16_t ports[] = {fixture.statePort, fixture.pointPort, fixture.imuPort};
                const uint16_t sourcePorts[] = {56200, 56300, 56400};
                const uint16_t keys[] = {kKeyStateInfoHostIpCfg, kKeyLidarPointDataHostIpCfg, kKeyLidarImuHostIpCfg};
                assert(command.length == 64);
                for (int i = 0; i < 3; ++i)
                {
                    const uint8_t *entry = command.data + 4 + i * 12;
                    assert(livox2::u16(entry) == keys[i] && livox2::u16(entry + 2) == 8);
                    assert(livox2::u32(entry + 4) == inet_addr("127.0.0.1"));
                    assert(livox2::u16(entry + 8) == ports[i]);
                    assert(livox2::u16(entry + 10) == sourcePorts[i]);
                }
                receivedHostConfig = true;
            }
            LIVOX2_CMD ack; ack.init(command.cmd_id, LVX2_CMD_ACK, command.seq_num); ack.sender_type = 1;
            ack.addData(uint8_t(0)); ack.addData(uint16_t(0));
            commands.send(wire(ack), outgoing.source);
        }
        assert(receivedHostConfig);
        await([&] { return Access::transport(driver).at("state") == lvxState_work; });

        const uint64_t stamp = 10000000000ULL;
        auto point = packet(1, 1, stamp, false);
        const LivoxLidarCartesianHighRawPoint raw{1000, 0, 0, 255, 0};
        memcpy(point.data() + 36, &raw, sizeof(raw));
        other.send(point, fixture.pointPort);
        sensor.send(imuBytes(stamp), fixture.pointPort); // Wrong data type for this port.
        sensor.send({1, 2, 3}, fixture.pointPort);
        auto oversized = packet(1, 96, stamp, false);
        oversized.resize(8192); // Its valid 1380-byte prefix must not survive datagram truncation.
        sensor.send(oversized, fixture.pointPort);
        sensor.send(point, fixture.pointPort);
        put(point, 28, stamp + 100000000, 8);
        sensor.send(point, fixture.pointPort);
        await([&] { return Access::transport(driver).at("pointPackets") == 2; });
        vector<GEOMETRY_POINT> actual;
        assert(fixture.cloud.get(actual) == stamp && actual.size() == 1);
        assert((actual[0].m_vP - Vector3f(1, 0, 0)).norm() < 1e-5f);

        other.send(imuBytes(stamp), fixture.imuPort);
        sensor.send(point, fixture.imuPort);
        sensor.send(imuBytes(stamp), fixture.imuPort);
        await([&] { return Access::transport(driver).at("imuSamples") == 1; });
        deque<IMUstream::IMU_DATA> gyro, acc;
        fixture.imu.get(gyro, acc);
        assert(gyro.size() == 1 && acc.size() == 1 && gyro[0].m_t == stamp);
        assert(std::abs(acc[0].m_v.z() - 9.80665f) < 1e-5f);
        assert(Access::transport(driver).at("outOfOrderPackets") == 0);
        assert(Access::transport(driver).at("invalidPackets").get<unsigned>() >= 4);

        LIVOX2_CMD push; push.init(LVX2_CMD_PUSH, LVX2_CMD_REQ, 0); push.sender_type = 1;
        push.addData(uint16_t(1)); push.addData(uint16_t(0));
        push.addData(uint16_t(kKeyPclDataType)); push.addData(uint16_t(1)); push.addData(uint8_t(3));
        sensor.send(wire(push), fixture.statePort);
        await([&] { return Access::transport(driver).at("observed").value("lvxPCLdataType", 0) == 3; });

        // Saving preserves automatic host selection, so moving to another NIC still works.
        assert(driver.saveConfig(false));
        assert(fixture.config.getJson()->at("lvxHostIP") == "0.0.0.0");
        driver.stop(); driver.stop();
        const auto stopped = Access::transport(driver);
        assert(stopped.at("closed") == true && stopped.at("pending") == 0 && stopped.at("framePoints") == 0);
        assert(driver.start());
        fixture.imu.get(gyro, acc); assert(gyro.empty() && acc.empty());
        const auto restarted = query.receive();
        assert(hostCommand(restarted.bytes, request));
        assert(request.cmd_id == LVX2_CMD_DISCOVER);
        assert(Access::transport(driver).at("state") == lvxState_deviceQuery);
        driver.stop();
        UdpEndpoint releasedCommand("127.0.0.1", fixture.commandPort);
        UdpEndpoint releasedState("127.0.0.1", fixture.statePort);
        UdpEndpoint releasedPoint("127.0.0.1", fixture.pointPort);
        UdpEndpoint releasedImu("127.0.0.1", fixture.imuPort);
    }

    void testNetworkValidationAndBindFailure()
    {
        UdpEndpoint query("127.0.0.2");
        DriverFixture fixture(query.port(), "127.0.0.1");
        _Livox2 &driver = fixture.driver;
        const json before = Access::values(driver);
        const json invalid[] = {
            {{"lvxIP", "0.0.0.0"}}, {{"lvxIP", "255.255.255.255"}}, {{"lvxIP", "239.1.2.3"}},
            {{"lvxHostIP", "not-an-ip"}}, {{"lvxPortQuery", 0}}, {{"lvxHostPortCmd", 65536}},
            {{"lvxHostPortPCL", 123.5}}, {{"receiveBufferBytes", -1}},
            {{"lvxHostPortPCL", fixture.imuPort}},
        };
        for (const auto &patch : invalid)
        {
            json errors;
            assert(!Access::offlineConfig(driver, patch, errors));
            assert(!errors.empty() && Access::values(driver) == before);
        }
        json errors;
        assert(!Access::config(driver, {{"lvxPortQuery", query.port() == 65535 ? 65534 : query.port() + 1}}, errors));
        assert(Access::values(driver) == before);

        // Fail after several sockets have opened; every earlier socket must be closed.
        UdpEndpoint occupied("127.0.0.1", fixture.pointPort);
        assert(!driver.start());
        assert(Access::transport(driver).at("closed") == true);
        {
            UdpEndpoint releasedCommand("127.0.0.1", fixture.commandPort);
            UdpEndpoint releasedState("127.0.0.1", fixture.statePort);
        }
        occupied.close();
        assert(driver.start());
        LIVOX2_CMD request;
        assert(hostCommand(query.receive().bytes, request));
        assert(request.cmd_id == LVX2_CMD_DISCOVER);
        driver.stop();
    }

    void testServiceTimers()
    {
        UdpEndpoint sensor("127.0.0.2");
        DriverFixture fixture(sensor.port());
        _Livox2 &driver = fixture.driver;
        assert(Access::open(driver));
        Access::commandPort(driver, sensor.port());
        Access::connected(driver);
        Access::queue(driver, kKeyDetectMode, 1);
        const auto original = sensor.receive();
        LIVOX2_CMD command;
        assert(hostCommand(original.bytes, command));
        const uint64_t sent = Access::sent(driver);
        Access::timers(driver, sent);
        Access::service(driver, sent + 499 * NSEC_MSEC);
        assert(sensor.empty() && Access::pending(driver) == 1);
        for (int attempt = 1; attempt <= 2; ++attempt)
        {
            Access::service(driver, sent + attempt * 500 * NSEC_MSEC);
            assert(sensor.receive().bytes == original.bytes); // Retries retain sequence and payload.
            assert(Access::pending(driver) == 1 && !Access::failed(driver));
        }
        Access::service(driver, sent + 1500 * NSEC_MSEC);
        assert(sensor.empty() && Access::pending(driver) == 0 && Access::failed(driver));
        assert(Access::transport(driver).at("commandRetries") == 2);
        assert(Access::transport(driver).at("commandErrors") == 1);

        // Exercise discovery pacing without waiting for the wall clock.
        driver.stop();
        assert(Access::open(driver));
        const uint64_t now = getTns();
        Access::service(driver, now);
        assert(hostCommand(sensor.receive().bytes, command) && command.cmd_id == LVX2_CMD_DISCOVER);
        const uint32_t sequence = command.seq_num;
        Access::service(driver, now + 500 * NSEC_MSEC);
        assert(sensor.empty());
        Access::service(driver, now + NSEC_SEC);
        assert(hostCommand(sensor.receive().bytes, command) && command.seq_num != sequence);

        // A transport timeout clears the old clock epoch, pending commands and IMU history.
        json errors;
        assert(Access::config(driver, {{"tOutSec", 1}}, errors));
        Access::commandPort(driver, sensor.port());
        Access::connected(driver);
        Access::queue(driver, kKeyDetectMode, 0); sensor.receive();
        Access::points(driver, points(driver, 1, 10000000000ULL));
        Access::imu(driver, imuPacket(driver, 10000000000ULL));
        Access::timers(driver, now);
        Access::service(driver, now + NSEC_SEC + 1);
        const auto reconnect = sensor.receive();
        assert(hostCommand(reconnect.bytes, command) && command.cmd_id == LVX2_CMD_DISCOVER);
        const auto reset = Access::transport(driver);
        assert(reset.at("state") == lvxState_deviceQuery && reset.at("pending") == 0 && reset.at("framePoints") == 0);
        assert(!Access::failed(driver));
        deque<IMUstream::IMU_DATA> gyro, acc; fixture.imu.get(gyro, acc);
        assert(gyro.empty() && acc.empty());
        vector<GEOMETRY_POINT> cloud; fixture.cloud.get(cloud); assert(cloud.empty());

        sensor.send(wire(discoveryReply(command.seq_num, sensor.port())), reconnect.source);
        Access::receiveQuery(driver);
        assert(Access::transport(driver).at("state") == lvxState_init && Access::pending(driver) == 6);
        for (int i = 0; i < 6; ++i)
            assert(hostCommand(sensor.receive().bytes, command) && command.cmd_id == LVX2_CMD_SET);
    }
}
int main()
{
    testProtocol(); testCloud(); testIMU(); testIMUPreview(); testIMUPreviewPersistence(); testConfigurationAndACK();
    testOwnedSockets(); testNetworkValidationAndBindFailure(); testServiceTimers();
    std::cout << "Livox2 packet, cloud, IMU, control, localhost UDP and socket lifecycle tests passed\n";
}
#endif
