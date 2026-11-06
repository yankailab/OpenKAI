#ifdef OPENKAI_LIVOX1_TEST
#include "../../src/Sensor/LiDAR/Livox/Livox1/_Livox1.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <poll.h>
#include <thread>

namespace kai
{
    struct Livox1TestAccess
    {
        static void outputs(_Livox1 &driver, PCLframe &cloud, IMUstream &imu)
        { driver.m_pPCLout = &cloud; driver.m_pIMUout = &imu; }
        static void transform(_Livox1 &driver, _TransformRigid &transform) { driver.m_pTf = &transform; }
        static bool config(_Livox1 &driver, const json &patch, json &errors, bool live = true)
        { return driver.applyConfig(patch, live, errors); }
        static json values(_Livox1 &driver) { return driver.configValues(); }
        static json schema(_Livox1 &driver) { return driver.controlSchema(); }
        static json preview(_Livox1 &driver) { return driver.imuValues(); }
        static void model(_Livox1 &driver, uint8_t type = livox1::Avia)
        {
            driver.m_lvxDevType = type; driver.m_firmwareKnown = true;
            driver.m_firmware = {{11, 6, 0, 0}};
        }
        static void connected(_Livox1 &driver) { driver.m_state = _Livox1::State::Connected; }
        static bool open(_Livox1 &driver) { string error; return driver.openSockets(error); }
        static void queue(_Livox1 &driver, const string &key) { driver.setLvxValue(key); }
        static void ack(_Livox1 &driver, const vector<uint8_t> &bytes)
        { assert(driver.recvLivoxCmd(bytes.data(), bytes.size())); driver.handleCtrlCmdAck(bytes.data(), bytes.size()); }
        static void points(_Livox1 &driver, const vector<uint8_t> &bytes)
        { assert(driver.recvLivoxData(bytes.data(), bytes.size())); driver.handlePointCloudData(bytes.data(), bytes.size()); }
        static void imu(_Livox1 &driver, const vector<uint8_t> &bytes)
        { assert(driver.recvLivoxData(bytes.data(), bytes.size())); driver.handleIMUdata(bytes.data(), bytes.size()); }
        static uint64_t timestamp(_Livox1 &driver, const vector<uint8_t> &bytes)
        { return driver.dataTimestamp(bytes.data()); }
        static void ppsElapsed(_Livox1 &driver, uint64_t elapsed) { driver.m_ppsAnchorHost -= elapsed; }
        static void receiveBroadcast(_Livox1 &driver) { driver.receive(_Livox1::Broadcast); }
        static void receiveCommand(_Livox1 &driver) { driver.receive(_Livox1::Command); }
        static void receivePoints(_Livox1 &driver) { driver.receive(_Livox1::PointCloud); }
        static void receiveImu(_Livox1 &driver) { driver.receive(_Livox1::Imu); }
        static void service(_Livox1 &driver, uint64_t now) { driver.service(now); }
        static void timers(_Livox1 &driver, uint64_t now)
        { driver.m_lastRx = driver.m_lastHeartbeat = driver.m_lastHandshake = now; }
        static vector<vector<uint8_t>> pending(_Livox1 &driver)
        {
            vector<vector<uint8_t>> result;
            for (const auto &entry : driver.m_pending) result.push_back(entry.second.bytes);
            return result;
        }
        static uint64_t sent(_Livox1 &driver) { return driver.m_pending.begin()->second.sent; }
        static json transport(_Livox1 &driver)
        {
            std::lock_guard<std::recursive_mutex> lock(driver.m_mutex);
            bool closed = true;
            for (int fd : driver.m_sockets) closed &= fd < 0;
            return {{"closed", closed}, {"connected", driver.m_state == _Livox1::State::Connected},
                {"initializing", driver.m_state == _Livox1::State::Initializing},
                {"discovering", driver.m_state == _Livox1::State::Discovering},
                {"failed", driver.m_configFailed}, {"pending", driver.m_pending.size()},
                {"pointPackets", driver.m_pointPackets}, {"imuSamples", driver.m_imuSamples},
                {"framePoints", driver.m_vFramePoints.size()}, {"invalidPackets", driver.m_invalidPackets},
                {"commandRetries", driver.m_commandRetries}, {"commandErrors", driver.m_commandErrors},
                {"observed", driver.m_observed}};
        }
    };
}

namespace
{
    using namespace kai;
    using Access = Livox1TestAccess;

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
            m_fd = ::socket(AF_INET, SOCK_DGRAM, 0); assert(m_fd >= 0);
            m_address.sin_family = AF_INET; m_address.sin_port = htons(port);
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
        void send(const vector<uint8_t> &bytes, uint16_t port)
        {
            sockaddr_in destination{};
            destination.sin_family = AF_INET; destination.sin_port = htons(port);
            destination.sin_addr.s_addr = inet_addr("127.0.0.1"); send(bytes, destination);
        }
        Datagram receive()
        {
            pollfd descriptor{m_fd, POLLIN, 0};
            assert(::poll(&descriptor, 1, 2000) == 1 && (descriptor.revents & POLLIN));
            Datagram datagram; datagram.bytes.resize(65536); socklen_t size = sizeof(datagram.source);
            const ssize_t count = ::recvfrom(m_fd, datagram.bytes.data(), datagram.bytes.size(), 0,
                reinterpret_cast<sockaddr *>(&datagram.source), &size);
            assert(count >= 0); datagram.bytes.resize(size_t(count)); return datagram;
        }
        bool empty() const { pollfd descriptor{m_fd, POLLIN, 0}; return ::poll(&descriptor, 1, 0) == 0; }
    private:
        int m_fd = -1;
        sockaddr_in m_address{};
    };

    struct DriverFixture
    {
        JsonCfg config;
        PCLframe cloud;
        IMUstream imu;
        _Livox1 driver;
        uint16_t broadcastPort, commandPort, pointPort, imuPort;
        explicit DriverFixture(uint16_t sensorPort)
        {
            UdpEndpoint broadcast, command, point, inertial;
            broadcastPort = broadcast.port(); commandPort = command.port();
            pointPort = point.port(); imuPort = inertial.port();
            config.setJson({{"name", "livox1-local-test"}, {"class", "_Livox1"},
                {"lvxIP", "127.0.0.2"}, {"lvxHostIP", "127.0.0.1"},
                {"lvxPortBroadcast", broadcastPort}, {"lvxPortCmd", sensorPort},
                {"lvxHostPortCmd", commandPort}, {"lvxHostPortPCL", pointPort}, {"lvxHostPortIMU", imuPort}});
            driver.BASE::setConfig(&config, config.getJson());
            assert(driver.loadConfig()); Access::outputs(driver, cloud, imu);
        }
    };

    vector<uint8_t> reply(const vector<uint8_t> &request, const vector<uint8_t> &payload = {0})
    {
        assert(livox1::validCommand(request.data(), request.size()));
        return livox1::makeCommand(request[9], request[10], 1, livox1::u16(request.data() + 5), payload);
    }

    void put(vector<uint8_t> &bytes, size_t offset, uint64_t value, size_t size)
    { for (size_t i = 0; i < size; ++i) bytes.at(offset + i) = uint8_t(value >> (8 * i)); }

    vector<uint8_t> packet(uint8_t type, uint64_t stamp = 1000000000ULL)
    {
        // Packet dimensions come from section 3.3 of Livox's legacy protocol,
        // independently of the implementation's pointSize/pointCount helpers.
        const size_t sizes[] = {13, 9, 14, 10, 28, 16, 24, 42, 22};
        const size_t counts[] = {100, 100, 96, 96, 48, 48, 1, 30, 30};
        assert(type < 9);
        vector<uint8_t> bytes(18 + sizes[type] * counts[type]);
        bytes[0] = 5; bytes[1] = bytes[2] = 1; bytes[9] = type;
        put(bytes, 10, stamp, 8);
        return bytes;
    }

    void testCommandGoldenVectors()
    {
        // CRC32 golden values independently calculated using zlib.crc32(data,
        // 0x564f580a), matching the seed inversion in Livox SDK FastCRCsw.cpp.
        const vector<uint8_t> handshake{
            0xaa, 0x01, 0x19, 0x00, 0x00, 0x34, 0x12, 0x8d, 0xba, 0x00, 0x01,
            0xc0, 0xa8, 0x01, 0x32, 0xc0, 0xda, 0xc1, 0xda, 0xc2, 0xda,
            0x59, 0x4a, 0xa2, 0xe6};
        const vector<uint8_t> heartbeat{
            0xaa, 0x01, 0x0f, 0x00, 0x00, 0x01, 0x00, 0xdc, 0xce, 0x00, 0x03,
            0x14, 0x60, 0xc6, 0x37};
        const vector<uint8_t> tripleReturn{
            0xaa, 0x01, 0x10, 0x00, 0x00, 0x02, 0x00, 0x08, 0x3a, 0x01, 0x06,
            0x03, 0xc9, 0xaa, 0x6f, 0xbf};
        assert(livox1::makeCommand(0, 1, 0, 0x1234,
            {192, 168, 1, 50, 0xc0, 0xda, 0xc1, 0xda, 0xc2, 0xda}) == handshake);
        assert(livox1::makeCommand(0, 3, 0, 1) == heartbeat);
        assert(livox1::makeCommand(1, 6, 0, 2, {3}) == tripleReturn);
        const uint8_t digits[] = {'1','2','3','4','5','6','7','8','9'};
        assert(livox1::crc32(digits, sizeof(digits)) == 0xb08db5b1U);
        for (const auto &bytes : {handshake, heartbeat, tripleReturn})
        {
            assert(livox1::validCommand(bytes.data(), bytes.size()));
            for (size_t n = 0; n < bytes.size(); ++n)
                assert(!livox1::validCommand(bytes.data(), n));
            for (size_t i = 0; i < bytes.size(); ++i)
            {
                auto corrupt = bytes; corrupt[i] ^= 1;
                assert(!livox1::validCommand(corrupt.data(), corrupt.size()));
            }
            auto trailing = bytes; trailing.push_back(0);
            assert(!livox1::validCommand(trailing.data(), trailing.size()));
        }
        assert(!livox1::validCommand(nullptr, 15));
        const auto largest = livox1::makeCommand(0, 11, 0, 1, vector<uint8_t>(1385));
        assert(largest.size() == 1400 && livox1::validCommand(largest.data(), largest.size()));
        bool bounded = false;
        try { livox1::makeCommand(0, 11, 0, 1, vector<uint8_t>(1386)); }
        catch (const std::length_error &) { bounded = true; }
        assert(bounded);
    }

    void testDataPacketsAndTimestamps()
    {
        for (uint8_t type = 0; type < 9; ++type)
        {
            auto bytes = packet(type);
            assert(livox1::validData(bytes.data(), bytes.size()));
            for (size_t n = 0; n < bytes.size(); ++n)
                assert(!livox1::validData(bytes.data(), n));
            auto trailing = bytes; trailing.push_back(0);
            assert(!livox1::validData(trailing.data(), trailing.size()));
            bytes[0] = 4; assert(!livox1::validData(bytes.data(), bytes.size()));
            bytes[0] = 5; bytes[9] = 9;
            assert(!livox1::validData(bytes.data(), bytes.size()));
        }
        assert(!livox1::validData(nullptr, 18));
        uint64_t stamp = 0;
        vector<uint8_t> utc{24, 2, 29, 12, 0, 0, 0, 0};
        put(utc, 4, 1234567, 4);
        assert(livox1::timestamp(utc.data(), 3, stamp));
        assert(stamp == 1709208001234567000ULL); // 2024-02-29T12:00:01.234567Z
        utc[0] = 23; assert(!livox1::timestamp(utc.data(), 3, stamp));
        utc[0] = 24; utc[1] = 13; assert(!livox1::timestamp(utc.data(), 3, stamp));
        utc[1] = 2; utc[3] = 24; assert(!livox1::timestamp(utc.data(), 3, stamp));
        utc[3] = 12; put(utc, 4, 3600000000ULL, 4);
        assert(!livox1::timestamp(utc.data(), 3, stamp));
        vector<uint8_t> raw(8); put(raw, 0, 123456789, 8);
        for (uint8_t type : {0, 1})
            assert(livox1::timestamp(raw.data(), type, stamp) && stamp == 123456789);
        // PPS timestamps are shifted by one second, leaving room for the
        // documented negative IMU compensation near a PPS rising edge.
        assert(livox1::timestamp(raw.data(), 4, stamp) && stamp == 1123456789);
        put(raw, 0, uint64_t(-1000000LL), 8);
        assert(livox1::timestamp(raw.data(), 4, stamp) && stamp == 999000000);
        put(raw, 0, 1000000000, 8);
        assert(livox1::timestamp(raw.data(), 4, stamp) && stamp == 2000000000);
        put(raw, 0, 1200000000, 8);
        assert(livox1::timestamp(raw.data(), 4, stamp) && stamp == 2200000000);
        put(raw, 0, uint64_t(INT64_MAX), 8);
        assert(livox1::timestamp(raw.data(), 4, stamp));
        assert(stamp == uint64_t(INT64_MAX) + 1000000000ULL);
        for (int64_t offset : {INT64_MIN, int64_t(-1000000000)})
        {
            put(raw, 0, uint64_t(offset), 8);
            assert(!livox1::timestamp(raw.data(), 4, stamp));
        }
        put(raw, 0, uint64_t(-999999999LL), 8);
        assert(livox1::timestamp(raw.data(), 4, stamp) && stamp == 1);
        assert(!livox1::timestamp(raw.data(), 2, stamp));
        assert(!livox1::timestamp(raw.data(), 5, stamp));
    }

    void testPointLayoutsAndTiming()
    {
        for (uint8_t type : {0, 2, 4, 7})
        {
            auto bytes = packet(type);
            const size_t echoes = type == 4 ? 2 : type == 7 ? 3 : 1;
            for (size_t echo = 0; echo < echoes; ++echo)
            {
                const size_t offset = 18 + 14 * echo;
                put(bytes, offset, 1000 * (echo + 1), 4);
                put(bytes, offset + 4, uint32_t(-2000), 4);
                put(bytes, offset + 8, 3000, 4);
                bytes[offset + 12] = uint8_t(200 + echo);
                if (type != 0) bytes[offset + 13] = uint8_t(0x10 * echo);
                livox1::Point point;
                assert(livox1::decodePoint(bytes.data() + 18, type, echo, point));
                assert(std::abs(point.x - (echo + 1)) < 1e-5f);
                assert(std::abs(point.y + 2) < 1e-5f && std::abs(point.z - 3) < 1e-5f);
                assert(point.reflectivity == 200 + echo);
                assert(point.tag == (type == 0 ? 0 : 0x10 * echo));
            }
            livox1::Point point;
            assert(!livox1::decodePoint(bytes.data() + 18, type, echoes, point));
        }
        for (uint8_t type : {1, 3, 5, 8})
        {
            auto bytes = packet(type);
            const bool multiple = type == 5 || type == 8;
            const size_t echoes = type == 5 ? 2 : type == 8 ? 3 : 1;
            put(bytes, 18 + (multiple ? 0 : 4), 9000, 2);
            put(bytes, 18 + (multiple ? 2 : 6), 9000, 2);
            for (size_t echo = 0; echo < echoes; ++echo)
            {
                const size_t offset = 18 + (multiple ? 4 + 6 * echo : 0);
                put(bytes, offset, 1000 * (echo + 1), 4);
                bytes[multiple ? offset + 4 : 26] = uint8_t(100 + echo);
                if (type != 1) bytes[multiple ? offset + 5 : 27] = uint8_t(0x10 * echo);
                livox1::Point point;
                assert(livox1::decodePoint(bytes.data() + 18, type, echo, point));
                assert(std::abs(point.x) < 1e-5f && std::abs(point.z) < 1e-5f);
                assert(std::abs(point.y - (echo + 1)) < 1e-5f);
                assert(point.reflectivity == 100 + echo);
                assert(point.tag == (type == 1 ? 0 : 0x10 * echo));
            }
            put(bytes, 18 + (multiple ? 0 : 4), 18001, 2);
            livox1::Point point;
            assert(!livox1::decodePoint(bytes.data() + 18, type, 0, point));
        }
        for (uint8_t device : {1, 6})
            assert(livox1::pointTime(100, device, 2, 95) == 950100);
        for (uint8_t device : {2, 3, 7})
        {
            assert(livox1::pointTime(100, device, 2, 6) == 25100);
            assert(livox1::pointTime(100, device, 2, 95) == 395933);
            for (uint8_t type : {4, 5})
            {
                assert(livox1::pointTime(100, device, type, 0) == 100);
                assert(livox1::pointTime(100, device, type, 1) == 100);
                assert(livox1::pointTime(100, device, type, 2) == 4266);
                assert(livox1::pointTime(100, device, type, 95) == 195933);
            }
        }
        for (uint8_t type : {7, 8})
        {
            assert(livox1::pointTime(100, 7, type, 2) == 100);
            assert(livox1::pointTime(100, 7, type, 3) == 4266);
            assert(livox1::pointTime(100, 7, type, 89) == 120933);
        }
    }

    vector<uint8_t> singlePoint(uint64_t stamp)
    {
        auto bytes = packet(2, stamp);
        put(bytes, 18, 1000, 4); bytes[30] = 255;
        return bytes;
    }

    vector<uint8_t> imuPacket(uint64_t stamp)
    {
        auto bytes = packet(6, stamp);
        const float values[] = {0.1f, -0.2f, 0.3f, 0.0f, 0.0f, 1.0f};
        for (size_t i = 0; i < 6; ++i)
        {
            uint32_t value; memcpy(&value, values + i, sizeof(value));
            put(bytes, 18 + 4 * i, value, 4);
        }
        return bytes;
    }

    void testOutputAdapters()
    {
        PCLframe cloud; IMUstream imu; _Livox1 driver; _TransformRigid transform;
        Access::outputs(driver, cloud, imu); Access::model(driver); Access::connected(driver);
        transform.setTranslation(5, -3, 2); Access::transform(driver, transform);
        const uint64_t stamp = 10000000000ULL;
        Access::points(driver, singlePoint(stamp));
        Access::points(driver, singlePoint(stamp)); // duplicates never enter the output frame
        Access::points(driver, singlePoint(stamp - 100));
        Access::points(driver, singlePoint(stamp + 100000000));
        vector<GEOMETRY_POINT> actual;
        assert(cloud.get(actual) == stamp && actual.size() == 1);
        assert(actual[0].m_tStamp == stamp);
        assert((actual[0].m_vP - Vector3f(6, -3, 2)).norm() < 1e-5f);
        assert(actual[0].m_vC.isApprox(Vector3f::Ones()));
        Access::imu(driver, imuPacket(stamp));
        Access::imu(driver, imuPacket(stamp));
        deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc); assert(gyro.size() == 1 && acc.size() == 1);
        assert(gyro[0].m_t == stamp && acc[0].m_t == stamp);
        assert(std::abs(gyro[0].m_v.y() + 0.2f) < 1e-5f);
        assert(std::abs(acc[0].m_v.z() - 9.80665f) < 1e-5f);
        assert(!Access::preview(driver).at("previewEnabled").get<bool>());
        for (unsigned i = 0; i < 3; ++i) Access::preview(driver);
        imu.get(gyro, acc); assert(gyro.size() == 1); // UI preview never consumes SLAM data
        json errors;
        assert(Access::config(driver, {{"bIMUpreview", true}}, errors));
        Access::imu(driver, imuPacket(stamp + 5000000));
        Access::imu(driver, imuPacket(stamp + 10000000));
        assert(Access::preview(driver).at("available") == true);
        assert(Access::preview(driver).at("orientationValid") == true);
        imu.get(gyro, acc); assert(gyro.size() == 3);
        assert(Access::config(driver, {{"lvxIMUdataEn", 0}}, errors));
        imu.get(gyro, acc); assert(gyro.empty() && acc.empty());
        driver.clear(); cloud.get(actual); assert(actual.empty());
    }

    void testAtomicConfiguration()
    {
        UdpEndpoint sensor("127.0.0.2"); DriverFixture fixture(sensor.port());
        auto &driver = fixture.driver; Access::model(driver);
        const auto before = Access::values(driver); json errors;
        const json invalid[] = {
            {{"lvxReturnMode", 4}, {"frameDurationMs", 25}},
            {{"lvxCoordinate", 2}}, {{"lvxScanPattern", -1}}, {{"lvxWorkMode", 0}},
            {{"lvxIP", "239.1.2.3"}}, {{"lvxHostIP", "not-an-ip"}},
            {{"lvxHostPortPCL", fixture.imuPort}}, {{"lvxPortCmd", 0}},
            {{"lvxHostPortIMU", 65536}}, {{"receiveBufferBytes", -1}},
            {{"vRangeD", {10, 1}}}, {{"vColorDefault", {1, "bad", 1}}},
            {{"bIMUpreview", 1}}, {{"bIMUstab", true}, {"frameDurationMs", 25}}
        };
        for (const auto &patch : invalid)
        {
            assert(!Access::config(driver, patch, errors, false));
            assert(!errors.empty() && Access::values(driver) == before);
        }
        assert(!Access::config(driver, {{"lvxHostIP", "127.0.0.3"}}, errors));
        assert(Access::values(driver) == before);
        assert(Access::config(driver, {{"lvxReturnMode", 3}, {"lvxScanPattern", 1},
            {"frameDurationMs", 25}, {"bIMUpreview", true}}, errors));
        assert(Access::values(driver).at("lvxReturnMode") == 3);
        assert(driver.saveConfig(false));
        _Livox1 restored; restored.BASE::setConfig(&fixture.config, fixture.config.getJson());
        assert(restored.loadConfig());
        assert(Access::values(restored).at("lvxReturnMode") == 3);
        assert(Access::values(restored).at("bIMUpreview") == true);
    }

    void testPersistentParameters()
    {
        UdpEndpoint sensor("127.0.0.2"); DriverFixture fixture(sensor.port());
        auto &driver = fixture.driver; Access::model(driver);
        assert(Access::schema(driver).at("lvxHighSensitivity").at("supported") == true);
        const auto before = Access::values(driver); json errors;
        assert(!Access::config(driver, {{"lvxHighSensitivity", 1}, {"lvxScanPattern", 1}}, errors));
        assert(Access::values(driver) == before);
        assert(Access::config(driver, {{"lvxHighSensitivity", false}, {"lvxScanPattern", 1}}, errors));
        assert(Access::open(driver));
        for (const auto &entry : vector<std::pair<string, uint8_t>>{{"lvxHighSensitivity", 1}, {"lvxScanPattern", 2}})
        {
            Access::queue(driver, entry.first);
            const auto command = sensor.receive().bytes;
            assert(command[9] == 0 && command[10] == 11 && command.size() == 20);
            assert(livox1::u16(command.data() + 11) == entry.second);
            assert(livox1::u16(command.data() + 13) == 1);
            assert(command[15] == (entry.second == 2 ? 1 : 0));
            Access::ack(driver, reply(command, {0}));
            assert(Access::pending(driver).size() == 1); // parameter ACKs include key and error fields
            Access::ack(driver, reply(command, {0, 0, 0, 0}));
            assert(Access::pending(driver).empty());
        }
        assert(Access::transport(driver).at("observed").at("lvxHighSensitivity") == false);
        assert(Access::transport(driver).at("observed").at("lvxScanPattern") == 1);
        Access::model(driver, livox1::Horizon);
        assert(Access::schema(driver).at("lvxHighSensitivity").at("supported") == false);
        assert(Access::schema(driver).at("lvxScanPattern").at("supported") == false);
        assert(!Access::config(driver, {{"lvxHighSensitivity", true}}, errors));
    }

    void testSharedPpsClock()
    {
        _Livox1 driver;
        auto point = singlePoint(999000000);
        point[8] = 4;
        assert(Access::timestamp(driver, point) == 1999000000);
        put(point, 10, 1000000, 8);
        assert(Access::timestamp(driver, point) == 2001000000); // PPS edge
        auto inertial = imuPacket(uint64_t(-1000000LL)); inertial[8] = 4;
        assert(Access::timestamp(driver, inertial) == 1999000000); // compensated IMU from before the edge
        put(inertial, 10, 2000000, 8);
        assert(Access::timestamp(driver, inertial) == 2002000000);
        Access::ppsElapsed(driver, 3 * NSEC_SEC);
        assert(Access::timestamp(driver, point) == 5001000000); // idle spans multiple PPS periods
    }

    void testPpsLossAndArithmeticLimits()
    {
        _Livox1 driver;
        auto point = singlePoint(1200000000); point[8] = 4;
        assert(livox1::validData(point.data(), point.size()));
        assert(Access::timestamp(driver, point) == 2200000000);
        // During PPS loss the device offset keeps increasing instead of wrapping.
        Access::ppsElapsed(driver, NSEC_SEC);
        put(point, 10, 2200000000, 8);
        assert(livox1::validData(point.data(), point.size()));
        assert(Access::timestamp(driver, point) == 3200000000);
        auto inertial = imuPacket(2199000000); inertial[8] = 4;
        assert(Access::timestamp(driver, inertial) == 3199000000);

        _Livox1 extreme;
        put(point, 10, uint64_t(INT64_MAX), 8);
        assert(livox1::validData(point.data(), point.size()));
        assert(Access::timestamp(extreme, point) == uint64_t(INT64_MAX) + NSEC_SEC);
        // This difference exceeded int64 in the former signed subtraction.
        put(inertial, 10, uint64_t(-999999999LL), 8);
        const auto reconstructed = Access::timestamp(extreme, inertial);
        assert(reconstructed > uint64_t(INT64_MAX));
        assert(reconstructed < uint64_t(INT64_MAX) + 2 * NSEC_SEC);
        put(inertial, 10, uint64_t(INT64_MIN), 8);
        assert(!livox1::validData(inertial.data(), inertial.size()));
        assert(Access::timestamp(extreme, inertial) == 0);

        // Force an elapsed duration that would exceed the unsigned output clock.
        // The shared-clock mapper must reject it before conversion to uint64.
        Access::ppsElapsed(extreme, UINT64_MAX / 2 + 20 * NSEC_SEC);
        assert(Access::timestamp(extreme, point) == 0);

        _Livox1 beforeEpoch;
        put(inertial, 10, uint64_t(-999999999LL), 8);
        assert(Access::timestamp(beforeEpoch, inertial) == 1);
        put(point, 10, 0, 8);
        assert(Access::timestamp(beforeEpoch, point) == 0); // inferred timestamp before the epoch
    }

    void testAcknowledgementsAndRetries()
    {
        UdpEndpoint sensor("127.0.0.2"); DriverFixture fixture(sensor.port());
        auto &driver = fixture.driver;
        assert(Access::open(driver)); Access::model(driver); Access::connected(driver);
        Access::queue(driver, "lvxReturnMode");
        const auto first = sensor.receive().bytes;
        assert(first[9] == 1 && first[10] == 6 && first[11] == 0);
        const uint16_t sequence = livox1::u16(first.data() + 5);
        Access::ack(driver, livox1::makeCommand(1, 6, 1, sequence - 1, {0}));
        Access::ack(driver, livox1::makeCommand(1, 8, 1, sequence, {0}));
        Access::ack(driver, reply(first, {}));
        assert(Access::pending(driver).size() == 1); // stale, wrong-command and short ACKs ignored
        Access::ack(driver, reply(first, {1}));
        assert(Access::pending(driver).empty() && Access::transport(driver).at("failed") == true);

        Access::queue(driver, "lvxCoordinate");
        const auto command = sensor.receive().bytes;
        const auto sent = Access::sent(driver);
        Access::timers(driver, sent + 499 * NSEC_MSEC);
        Access::service(driver, sent + 499 * NSEC_MSEC);
        assert(sensor.empty());
        for (unsigned attempt = 1; attempt <= 2; ++attempt)
        {
            const auto now = sent + attempt * 500 * NSEC_MSEC;
            Access::timers(driver, now); Access::service(driver, now);
            assert(sensor.receive().bytes == command); // retry preserves payload and sequence
        }
        Access::timers(driver, sent + 1500 * NSEC_MSEC);
        Access::service(driver, sent + 1500 * NSEC_MSEC);
        assert(sensor.empty() && Access::pending(driver).empty());
        assert(Access::transport(driver).at("commandRetries") == 2);
        assert(Access::transport(driver).at("commandErrors") == 2);
        driver.stop();
    }

    vector<uint8_t> broadcast(uint8_t model = livox1::Avia)
    {
        vector<uint8_t> payload(19);
        memcpy(payload.data(), "LOCAL-AVIA-001", 14); payload[16] = model;
        return livox1::makeCommand(0, 0, 2, 1, payload);
    }

    void testLocalhostLifecycle()
    {
        UdpEndpoint sensor("127.0.0.2"), other("127.0.0.3");
        DriverFixture fixture(sensor.port()); auto &driver = fixture.driver;
        assert(Access::open(driver));
        other.send(broadcast(), fixture.broadcastPort); Access::receiveBroadcast(driver);
        assert(sensor.empty());
        auto badBroadcast = broadcast(); badBroadcast.back() ^= 1;
        sensor.send(badBroadcast, fixture.broadcastPort); Access::receiveBroadcast(driver);
        assert(sensor.empty());
        sensor.send(broadcast(), fixture.broadcastPort); Access::receiveBroadcast(driver);
        const auto handshake = sensor.receive();
        assert(livox1::validCommand(handshake.bytes.data(), handshake.bytes.size()));
        assert(handshake.bytes[4] == 1 && handshake.bytes[9] == 0 && handshake.bytes[10] == 1);
        assert(handshake.bytes.size() == 25);
        assert(livox1::u32(handshake.bytes.data() + 11) == inet_addr("127.0.0.1"));
        assert(livox1::u16(handshake.bytes.data() + 15) == fixture.pointPort);
        assert(livox1::u16(handshake.bytes.data() + 17) == fixture.commandPort);
        assert(livox1::u16(handshake.bytes.data() + 19) == fixture.imuPort);
        assert(ntohs(handshake.source.sin_port) == fixture.commandPort);
        sensor.send(reply(handshake.bytes), handshake.source); Access::receiveCommand(driver);
        for (unsigned i = 0; i < 2; ++i)
        {
            const auto request = sensor.receive();
            assert(request.bytes[9] == 0 && (request.bytes[10] == 2 || request.bytes[10] == 3));
            sensor.send(reply(request.bytes, request.bytes[10] == 2 ?
                vector<uint8_t>{0, 11, 6, 0, 0} : vector<uint8_t>{0, 0, 0, 0, 0, 0, 0}), request.source);
            Access::receiveCommand(driver);
        }
        assert(Access::transport(driver).at("initializing") == true);
        assert(Access::pending(driver).empty() && sensor.empty());
        // A still-initializing device must receive heartbeat, never an early sampling request.
        Access::service(driver, getTns() + NSEC_SEC);
        const auto heartbeat = sensor.receive();
        assert(heartbeat.bytes[9] == 0 && heartbeat.bytes[10] == 3);
        sensor.send(reply(heartbeat.bytes, {0, 5, 0, 0, 0, 0, 0}), heartbeat.source);
        Access::receiveCommand(driver);
        assert(Access::pending(driver).size() == 1 && Access::transport(driver).at("initializing") == true);
        sensor.send(reply(heartbeat.bytes, {0, 1, 0, 0, 0, 0, 0}), heartbeat.source);
        Access::receiveCommand(driver);
        unsigned configs = 0, sampling = 0;
        while (!sensor.empty())
        {
            const auto request = sensor.receive();
            const auto &bytes = request.bytes;
            vector<uint8_t> response{0};
            if (bytes[9] == 0 && bytes[10] == 4)
            {
                assert(configs == 5 && bytes[11] == 1); ++sampling;
            }
            else
            {
                ++configs;
                if (bytes[9] == 1 && bytes[10] == 7) response = {0, 0};
                else if (bytes[9] == 1 && bytes[10] == 9) response = {0, 1};
                else if (bytes[9] == 0 && bytes[10] == 12)
                {
                    assert(bytes.size() == 18 && bytes[11] == 1 && bytes[13] == 0);
                    const uint8_t key = bytes[12];
                    assert(key == 1 || key == 2);
                    response = {0, 0, 0, 0, key, 0, 1, 0, uint8_t(key == 1)};
                }
                else assert(bytes[9] == 0 && bytes[10] == 5); // Avia has no fan
            }
            sensor.send(reply(bytes, response), request.source); Access::receiveCommand(driver);
            assert(configs <= 5 && sampling <= 1);
        }
        assert(sampling == 1 && Access::transport(driver).at("connected") == true);
        assert(Access::transport(driver).at("failed") == false);

        const uint64_t stamp = 10000000000ULL;
        other.send(singlePoint(stamp), fixture.pointPort);
        sensor.send(imuPacket(stamp), fixture.pointPort);
        sensor.send({1, 2, 3}, fixture.pointPort);
        auto oversized = singlePoint(stamp); oversized.resize(8192);
        sensor.send(oversized, fixture.pointPort); // MSG_TRUNC must reject a valid prefix
        sensor.send(singlePoint(stamp), fixture.pointPort);
        sensor.send(singlePoint(stamp + 100000000), fixture.pointPort);
        Access::receivePoints(driver);
        assert(Access::transport(driver).at("pointPackets") == 2);
        vector<GEOMETRY_POINT> points;
        assert(fixture.cloud.get(points) == stamp && points.size() == 1);
        assert(points[0].m_vP.isApprox(Vector3f(1, 0, 0)));
        other.send(imuPacket(stamp), fixture.imuPort);
        sensor.send(singlePoint(stamp), fixture.imuPort);
        sensor.send(imuPacket(stamp), fixture.imuPort); Access::receiveImu(driver);
        assert(Access::transport(driver).at("imuSamples") == 1);
        assert(Access::transport(driver).at("invalidPackets").get<unsigned>() >= 5);

        // Loss of the control connection discards the old sensor clock epoch.
        json errors; assert(Access::config(driver, {{"tOutSec", 2}}, errors));
        const auto now = getTns(); Access::timers(driver, now);
        Access::service(driver, now + 2 * NSEC_SEC + 1);
        assert(Access::transport(driver).at("discovering") == true);
        assert(Access::pending(driver).empty() && Access::transport(driver).at("framePoints") == 0);
        deque<IMUstream::IMU_DATA> gyro, acc; fixture.imu.get(gyro, acc);
        assert(gyro.empty() && acc.empty());
        fixture.cloud.get(points); assert(points.empty());
        sensor.send(broadcast(), fixture.broadcastPort); Access::receiveBroadcast(driver);
        const auto newHandshake = sensor.receive();
        assert(newHandshake.bytes[10] == 1);
        assert(livox1::u16(newHandshake.bytes.data() + 5) != livox1::u16(handshake.bytes.data() + 5));
        driver.stop(); driver.stop();
        assert(Access::transport(driver).at("closed") == true && Access::pending(driver).empty());
        UdpEndpoint releasedCommand("127.0.0.1", fixture.commandPort);
        UdpEndpoint releasedPoint("127.0.0.1", fixture.pointPort);
        UdpEndpoint releasedImu("127.0.0.1", fixture.imuPort);
    }

    void testBindFailureAndWorkerShutdown()
    {
        UdpEndpoint sensor("127.0.0.2"); DriverFixture fixture(sensor.port());
        auto &driver = fixture.driver;
        UdpEndpoint occupied("127.0.0.1", fixture.pointPort);
        assert(!driver.start());
        assert(Access::transport(driver).at("closed") == true);
        { UdpEndpoint releasedCommand("127.0.0.1", fixture.commandPort); }
        occupied.close();
        assert(driver.start());
        sensor.send(broadcast(), fixture.broadcastPort);
        const auto handshake = sensor.receive();
        assert(handshake.bytes[9] == 0 && handshake.bytes[10] == 1);
        driver.stop();
        assert(Access::transport(driver).at("closed") == true);
        assert(driver.start()); // starting the worker again does not reuse pending requests
        sensor.send(broadcast(), fixture.broadcastPort);
        const auto restarted = sensor.receive();
        assert(restarted.bytes[9] == 0 && restarted.bytes[10] == 1);
        assert(livox1::u16(restarted.bytes.data() + 5) != livox1::u16(handshake.bytes.data() + 5));
        driver.stop();
    }
}

int main()
{
    testCommandGoldenVectors();
    testDataPacketsAndTimestamps();
    testPointLayoutsAndTiming();
    testOutputAdapters();
    testAtomicConfiguration();
    testPersistentParameters();
    testSharedPpsClock();
    testPpsLossAndArithmeticLimits();
    testAcknowledgementsAndRetries();
    testLocalhostLifecycle();
    testBindFailureAndWorkerShutdown();
    std::cout << "Livox1 protocol and driver regression tests passed\n";
}
#endif
