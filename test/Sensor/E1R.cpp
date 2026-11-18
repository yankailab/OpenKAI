#ifdef OPENKAI_E1R_TEST
#include "../../src/Sensor/LiDAR/RoboSense/E1R/_E1R.h"
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <thread>

namespace kai
{
    struct E1RTestAccess
    {
        static void output(_E1R &driver, PCLstream &cloud) { driver.m_pPCLout = &cloud; }
        static void packet(_E1R &driver, const e1r::Msop &packet, uint64_t received = 100000000000ULL)
        { driver.handleMSOP(packet, received); }
        static void info(_E1R &driver, const e1r::Difop &packet) { driver.handleDIFOP(packet, getTns()); }
        static bool config(_E1R &driver, const json &patch, json &errors)
        { return driver.applyConfig(patch, true, errors); }
        static json config(_E1R &driver) { return driver.configValues(); }
        static json schema(_E1R &driver) { return driver.controlSchema(); }
        static json status(_E1R &driver) { return driver.statusValues(); }
        static json transport(_E1R &driver)
        {
            std::lock_guard<std::recursive_mutex> lock(driver.m_mutex);
            return {{"closed", driver.m_msopSocket < 0 && driver.m_difopSocket < 0},
                {"ready", driver.m_msopSocket >= 0 && driver.m_difopSocket >= 0},
                {"error", driver.m_lastError}, {"invalidPackets", driver.m_invalidPackets}};
        }
    };
}

namespace
{
    using namespace kai;
    using Access = E1RTestAccess;

    class DriverWithTransform : public _E1R
    {
    public:
        DriverWithTransform() { m_pTf = &m_transform; }
        ~DriverWithTransform() { stop(); }
        _TransformRigid m_transform;
    };

    class InspectableStream : public PCLstream
    {
    public:
        GEOMETRY_POINT newest()
        {
            std::shared_lock lock(m_sMutex);
            assert(!m_vElement.empty());
            return m_vElement[(m_iBset + m_vElement.size() - 1) % m_vElement.size()];
        }
    };

    class UdpEndpoint
    {
    public:
        explicit UdpEndpoint(uint16_t port = 0)
        {
            m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
            assert(m_fd >= 0);
            m_address.sin_family = AF_INET;
            m_address.sin_port = htons(port);
            m_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            assert(::bind(m_fd, reinterpret_cast<sockaddr *>(&m_address), sizeof(m_address)) == 0);
            socklen_t size = sizeof(m_address);
            assert(::getsockname(m_fd, reinterpret_cast<sockaddr *>(&m_address), &size) == 0);
        }
        ~UdpEndpoint() { close(); }
        UdpEndpoint(const UdpEndpoint &) = delete;
        UdpEndpoint &operator=(const UdpEndpoint &) = delete;
        uint16_t port() const { return ntohs(m_address.sin_port); }
        void close() { if (m_fd >= 0) ::close(m_fd); m_fd = -1; }
        void send(uint16_t port)
        {
            sockaddr_in destination = m_address;
            destination.sin_port = htons(port);
            const uint8_t invalidPacket = 0;
            assert(::sendto(m_fd, &invalidPacket, sizeof(invalidPacket), 0,
                reinterpret_cast<sockaddr *>(&destination), sizeof(destination)) == sizeof(invalidPacket));
        }
    private:
        int m_fd = -1;
        sockaddr_in m_address{};
    };

    template <class Predicate> void await(Predicate predicate)
    {
        const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!predicate() && std::chrono::steady_clock::now() < end)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        assert(predicate());
    }

    void testSocketRetries()
    {
        // Block the second socket so a failed attempt must release the first bind.
        UdpEndpoint msop, difop;
        const uint16_t msopPort = msop.port(), difopPort = difop.port();
        JsonCfg config;
        config.setJson({{"name", "e1r-retry-test"}, {"class", "_E1R"},
            {"deviceIP", "127.0.0.1"}, {"bindIP", "127.0.0.1"},
            {"msopPort", msopPort}, {"difopPort", difopPort}});
        PCLstream cloud;
        DriverWithTransform driver;
        driver.setConfig(&config, config.getJson());
        assert(driver.loadConfig());
        Access::output(driver, cloud);
        msop.close();
        assert(driver.start());
        await([&] { return !Access::transport(driver).at("error").get<string>().empty(); });
        assert(Access::transport(driver).at("closed") == true);
        {
            UdpEndpoint releasedMSOP(msopPort);
        }
        difop.close();
        await([&] { return Access::transport(driver).at("ready") == true; });
        assert(Access::transport(driver).at("error") == "");
        UdpEndpoint sender;
        sender.send(msopPort);
        await([&] { return Access::transport(driver).at("invalidPackets") == 1; });
        driver.stop();
        assert(Access::transport(driver).at("closed") == true);

        {
            UdpEndpoint blockedDIFOP(difopPort);
            assert(driver.start());
            await([&] { return !Access::transport(driver).at("error").get<string>().empty(); });
            const auto stopping = std::chrono::steady_clock::now();
            driver.stop();
            assert(std::chrono::steady_clock::now() - stopping < std::chrono::milliseconds(500));
            assert(Access::transport(driver).at("closed") == true);
        }
        assert(driver.start());
        await([&] { return Access::transport(driver).at("ready") == true; });
        driver.stop();
        UdpEndpoint releasedMSOP(msopPort), releasedDIFOP(difopPort);
    }

    e1r::Msop packet(uint16_t sequence, uint64_t timestampUs, size_t count = 1)
    {
        e1r::Msop result;
        result.sequence = sequence;
        result.timestampUs = timestampUs;
        result.returnMode = 4;
        for (size_t i = 0; i < count; ++i)
        {
            auto &point = result.points[i];
            point.x = point.range = 1;
            point.intensity = 128;
            point.attribute = 1;
            point.offsetUs = static_cast<uint16_t>(i);
        }
        return result;
    }

    void configure(_E1R &driver, const json &config)
    {
        json errors;
        assert(Access::config(driver, config, errors));
        assert(errors.empty());
    }

    void testImmediatePacketsAndSequence()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        Access::output(driver, cloud);
        const uint64_t before = getTns();
        Access::packet(driver, packet(0, 1000000));
        vector<GEOMETRY_POINT> points;
        const uint64_t update = cloud.get(points, 0);
        assert(update >= before && update <= getTns());
        assert(points.size() == 1 && points[0].m_tStamp == 1000000000ULL);
        Access::packet(driver, packet(1, 1000100));
        Access::packet(driver, packet(1, 1000100)); // Duplicate diagnostics do not filter points.
        Access::packet(driver, packet(0, 1000000)); // Reordered capture times are retained.
        Access::packet(driver, packet(3, 1000300)); // Packet 2 was lost.
        Access::packet(driver, packet(0, 1100000));
        cloud.get(points, 0);
        assert(points.size() == 6);
        assert(points[0].m_tStamp == 1000000000ULL && points[3].m_tStamp == 1000000000ULL);
        json status = Access::status(driver);
        assert(status.at("pointPackets") == 6 && status.at("points") == 6);
        assert(status.at("droppedPackets") == 1 && status.at("outOfOrderPackets") == 2);
        assert(!status.contains("frames") && !status.contains("partialFrames") && !status.contains("frameOverflows"));
        Access::packet(driver, packet(1, 1100100));
        Access::packet(driver, packet(0, 1200000));
        cloud.get(points, 1100000000ULL);
        assert(points.size() == 2 && points[0].m_tStamp == 1100100000ULL && points[1].m_tStamp == 1200000000ULL);
    }

    void testSequenceResetAndMissingPrefix()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        Access::output(driver, cloud);
        Access::packet(driver, packet(0, 1000000));
        Access::packet(driver, packet(0, 1000000));
        Access::packet(driver, packet(0, 1100000));
        vector<GEOMETRY_POINT> points;
        cloud.get(points, 0);
        assert(points.size() == 3 && Access::status(driver).at("outOfOrderPackets") == 1);
        Access::packet(driver, packet(3, 1100300));
        assert(Access::status(driver).at("droppedPackets") == 2);
        Access::packet(driver, packet(2, 1200200)); // Lost prefix of next scan.
        assert(Access::status(driver).at("droppedPackets") == 4);
        cloud.get(points, 0);
        assert(points.size() == 5 && points.back().m_tStamp == 1200200000ULL);
        _E1R lateStart;
        Access::packet(lateStart, packet(5, 1000000));
        assert(Access::status(lateStart).at("droppedPackets") == 5);
    }

    void testFilteringAndTransform()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        Access::output(driver, cloud);
        configure(driver, {{"vRangeD", {1.0, 10.0}}});
        driver.m_transform.setTranslation(1, 2, 3);
        driver.m_transform.setRotation(0, 0, 0.5, 0.5);
        auto data = packet(0, 1000000, 8);
        data.points[0].x = data.points[0].range = 0.5f; // Outside near boundary.
        data.points[2].x = data.points[2].range = 10.0f; // Inclusive far boundary.
        data.points[3].x = data.points[3].range = 11.0f; // Outside far boundary.
        data.points[4].attribute = 2; // Noisy.
        data.points[5].attribute = 0; // Valid unclassified output from live firmware 8.1.1.
        data.points[6].attribute = 3; // Unknown attributes remain filtered.
        data.points[7].attribute = 255;
        Access::packet(driver, data);
        vector<GEOMETRY_POINT> points;
        cloud.get(points, 0);
        assert(points.size() == 3);
        assert((points[0].m_vP - Vector3f(1, 3, 3)).norm() < 0.0001f);
        assert((points[1].m_vP - Vector3f(1, 12, 3)).norm() < 0.0001f);
        assert((points[0].m_vC - Vector3f::Constant(128.0f / 255)).norm() < 0.0001f);
        assert(points[0].m_tStamp == 1000001000ULL);
        assert(points[2].m_tStamp == 1000005000ULL); // Attribute 0 survives alongside normal 1.
        configure(driver, {{"bReflectivity", false}, {"bDropNoise", false}, {"vColorDefault", {0.2, 0.4, 0.6}}});
        cloud.clear();
        Access::packet(driver, data);
        cloud.get(points, 0);
        assert(points.size() == 6); // Disabling the filter also retains noise and unknown attributes.
        assert((points[0].m_vC - Vector3f(0.2f, 0.4f, 0.6f)).norm() < 0.0001f);
    }

    void testCapacityAndClear()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        Access::output(driver, cloud);
        assert(cloud.clear(96)); // Capacity belongs to the output stream.
        Access::packet(driver, packet(0, 1000000, 96));
        Access::packet(driver, packet(1, 1000100, 96));
        vector<GEOMETRY_POINT> points;
        cloud.get(points, 0);
        assert(points.size() == 96);
        assert(points.front().m_tStamp == 1000100000 && points.back().m_tStamp == 1000195000);
        assert(Access::status(driver).at("points") == 192);
        Access::packet(driver, packet(0, 1100000));
        cloud.get(points, 0);
        assert(points.size() == 96 && points.front().m_tStamp == 1000101000 && points.back().m_tStamp == 1100000000);
        driver.clear();
        cloud.get(points, 0);
        assert(points.empty());
        Access::packet(driver, packet(0, 100));
        cloud.get(points, 0);
        assert(points.size() == 1 && points[0].m_tStamp == 100000);
        driver.stop(); driver.stop();
        assert(Access::status(driver).at("state") == "stopped");
        cloud.get(points, 0);
        assert(points.size() == 1); // Stopping a driver does not erase consumer history.
    }

    void testClockAndOverflow()
    {
        DriverWithTransform driver;
        InspectableStream cloud;
        Access::output(driver, cloud);
        Access::packet(driver, packet(0, 3000000));
        Access::packet(driver, packet(1, 3000100));
        Access::packet(driver, packet(0, 1000000));
        assert(Access::status(driver).at("clockResets") == 1);
        auto changedTimeMode = packet(0, 2000000);
        changedTimeMode.timeMode = 3;
        Access::packet(driver, changedTimeMode);
        assert(Access::status(driver).at("clockResets") == 2);
        vector<GEOMETRY_POINT> points;
        cloud.get(points, 0);
        assert(points.size() == 4 && points[2].m_tStamp == 1000000000);
        configure(driver, {{"bUseSensorTime", false}});
        cloud.clear();
        Access::packet(driver, packet(0, UINT64_MAX, 2), 5000000000ULL);
        cloud.get(points, 0);
        // Host timestamps remain usable when converting the sensor clock would overflow.
        assert(points.size() == 2 && points[0].m_tStamp == 5000000000ULL && points[1].m_tStamp == 5000001000ULL);
        configure(driver, {{"bUseSensorTime", true}});
        cloud.clear();
        const uint64_t beforeInvalid = cloud.get(points, 0);
        Access::packet(driver, packet(0, UINT64_MAX / 1000 + 1));
        assert(cloud.get(points, 0) == beforeInvalid && points.empty());
        assert(Access::status(driver).at("invalidPackets") == 1);
        configure(driver, {{"bUseSensorTime", false}});
        Access::packet(driver, packet(0, 1, 2), UINT64_MAX - 10);
        assert(cloud.get(points, 0) == beforeInvalid && points.empty());
        assert(Access::status(driver).at("invalidPackets") == 2);
        configure(driver, {{"bUseSensorTime", true}});
        Access::packet(driver, packet(0, 0));
        assert(cloud.newest().m_tStamp == 0 && cloud.newest().m_vP.x() == 1);
        cloud.get(points, 0);
        assert(points.empty()); // The reader chooses to omit invalid timestamps.
    }

    void testConfigAtomicityAndTelemetry()
    {
        DriverWithTransform driver;
        json errors;
        const json before = Access::config(driver);
        PCLstream cloud;
        Access::output(driver, cloud);
        Access::packet(driver, packet(0, 1000000));
        vector<GEOMETRY_POINT> points;
        const uint64_t update = cloud.get(points, 0);
        const json poseFields = {{"vPos", {1, 2, 3}}, {"vOrt", {0, 0, 0, 1}},
                                 {"vT", {1, 2, 3}}, {"vR", {0, 0, 0, 1}}};
        for (const auto &field : poseFields.items())
        {
            assert(!before.contains(field.key()) && !Access::schema(driver).contains(field.key()));
            assert(!Access::config(driver, {{field.key(), field.value()}}, errors));
            assert(errors.at(field.key()) == "Unknown parameter" && Access::config(driver) == before);
            assert(cloud.get(points, 0) == update && points.size() == 1);
        }
        const json invalid[] = {
            json(), json::array(),
            {{"msopPort", 12345}, {"vRangeD", {10.0, 1.0}}},
            {{"msopPort", 7788}}, {{"difopPort", -1}}, {{"msopPort", 1.5}},
            {{"deviceIP", "192.168.1.999"}}, {{"bindIP", false}},
            {{"bDropNoise", "true"}}, {{"nMaxFramePoints", 96}}, {{"frameTimeoutMs", 250}},
            {{"vColorDefault", {0, 0, 1.1}}}, {{"vRangeD", {0, std::numeric_limits<double>::infinity()}}},
            {{"unknown", 1}}, {{"devicePHYmode", 2}},
        };
        for (const auto &patch : invalid)
        {
            assert(!Access::config(driver, patch, errors));
            assert(!errors.empty() && Access::config(driver) == before);
            assert(cloud.get(points, 0) == update && points.size() == 1); // Failed config preserves stored points.
        }
        configure(driver, {{"vRangeD", {0.0, 20.0}}});
        assert(cloud.get(points, 0) == update && points.size() == 1); // Configuration does not erase consumer history.
        e1r::Difop info;
        info.sensorIp = "192.168.1.200";
        info.msopIp = "192.168.1.102";
        info.phyMode = 2;
        info.frameRate = 10;
        info.timestampUs = 123456789;
        Access::info(driver, info);
        const json values = Access::config(driver);
        assert(values.at("deviceSensorIP") == "192.168.1.200" && values.at("devicePHYmode") == 2);
        assert(Access::status(driver).at("deviceControlSupported") == false);
        assert(Access::status(driver).at("observed").at("timestampUs") == "123456789");
        configure(driver, values); // A complete web round-trip preserves unchanged read-only values.
        assert(!Access::config(driver, {{"devicePHYmode", 1}}, errors));
        assert(Access::config(driver).at("devicePHYmode") == 2);
    }
}

int main()
{
    testSocketRetries();
    testImmediatePacketsAndSequence();
    testSequenceResetAndMissingPrefix();
    testFilteringAndTransform();
    testCapacityAndClear();
    testClockAndOverflow();
    testConfigAtomicityAndTelemetry();
    std::cout << "E1R driver tests passed\n";
}
#endif
