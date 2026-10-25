#ifdef OPENKAI_E1R_TEST
#include "../../src/Sensor/LiDAR/RoboSense/E1R/_E1R.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

namespace kai
{
    struct E1RTestAccess
    {
        static void output(_E1R &driver, PCLframe &cloud) { driver.m_pPCLout = &cloud; }
        static void packet(_E1R &driver, const e1r::Msop &packet, uint64_t received = 100000000000ULL)
        { driver.handleMSOP(packet, received); }
        static void info(_E1R &driver, const e1r::Difop &packet) { driver.handleDIFOP(packet, getTns()); }
        static void publish(_E1R &driver, bool partial = false) { driver.publishFrame(partial); }
        static bool config(_E1R &driver, const json &patch, json &errors)
        { return driver.applyConfig(patch, true, errors); }
        static json config(_E1R &driver) { return driver.configValues(); }
        static json schema(_E1R &driver) { return driver.controlSchema(); }
        static json status(_E1R &driver) { return driver.statusValues(); }
        static size_t pending(_E1R &driver) { return driver.m_framePoints.size(); }
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
            point.timestampUs = timestampUs + i;
        }
        return result;
    }

    void configure(_E1R &driver, const json &config)
    {
        json errors;
        assert(Access::config(driver, config, errors));
        assert(errors.empty());
    }

    void testFrames()
    {
        DriverWithTransform driver;
        PCLframe cloud;
        Access::output(driver, cloud);
        Access::packet(driver, packet(0, 1000000));
        Access::packet(driver, packet(1, 1000100));
        Access::packet(driver, packet(1, 1000100)); // Exact duplicate.
        Access::packet(driver, packet(0, 1000000)); // Reordered older packet.
        Access::packet(driver, packet(3, 1000300)); // Packet 2 was lost.
        Access::packet(driver, packet(0, 1100000)); // Next frame publishes previous.
        vector<GEOMETRY_POINT> points;
        assert(cloud.get(points) == 1000000000ULL && points.size() == 3);
        assert(points[0].m_tStamp == 1000000000ULL && points[2].m_tStamp == 1000300000ULL);
        json status = Access::status(driver);
        assert(status.at("frames") == 1 && status.at("partialFrames") == 1);
        assert(status.at("droppedPackets") == 1 && status.at("outOfOrderPackets") == 2);
        Access::packet(driver, packet(1, 1100100));
        Access::packet(driver, packet(0, 1200000));
        assert(cloud.get(points) == 1100000000ULL && points.size() == 2);
        status = Access::status(driver);
        assert(status.at("frames") == 2 && status.at("partialFrames") == 1);
    }

    void testSinglePacketFrames()
    {
        DriverWithTransform driver;
        PCLframe cloud;
        Access::output(driver, cloud);
        Access::packet(driver, packet(0, 1000000));
        Access::packet(driver, packet(0, 1000000));
        Access::packet(driver, packet(0, 1100000));
        vector<GEOMETRY_POINT> points;
        assert(cloud.get(points) == 1000000000ULL && points.size() == 1);
        assert(Access::status(driver).at("outOfOrderPackets") == 1);
        assert(Access::status(driver).at("frames") == 1);
    }

    void testTimeoutAndPartialPrefix()
    {
        DriverWithTransform driver;
        PCLframe cloud;
        Access::output(driver, cloud);
        Access::packet(driver, packet(0, 1000000));
        Access::packet(driver, packet(1, 1000100));
        Access::publish(driver, true); // No further packets before frame timeout.
        Access::packet(driver, packet(3, 1000300)); // Continuation, not a new prefix.
        assert(Access::status(driver).at("droppedPackets") == 1);
        Access::packet(driver, packet(2, 1100200)); // Lost prefix of next frame.
        assert(Access::status(driver).at("droppedPackets") == 3);
        Access::publish(driver);
        vector<GEOMETRY_POINT> points;
        assert(cloud.get(points) == 1100200000ULL && points.size() == 1);
        assert(Access::status(driver).at("partialFrames") == 3);

        _E1R lateStart;
        Access::packet(lateStart, packet(5, 1000000));
        assert(Access::status(lateStart).at("droppedPackets") == 5);
        Access::publish(lateStart);
        assert(Access::status(lateStart).at("partialFrames") == 1);
    }

    void testFilteringAndTransform()
    {
        DriverWithTransform driver;
        PCLframe cloud;
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
        Access::publish(driver);
        vector<GEOMETRY_POINT> points;
        cloud.get(points);
        assert(points.size() == 3);
        assert((points[0].m_vP - Vector3f(1, 3, 3)).norm() < 0.0001f);
        assert((points[1].m_vP - Vector3f(1, 12, 3)).norm() < 0.0001f);
        assert((points[0].m_vC - Vector3f::Constant(128.0f / 255)).norm() < 0.0001f);
        assert(points[0].m_tStamp == 1000001000ULL);
        assert(points[2].m_tStamp == 1000005000ULL); // Attribute 0 survives alongside normal 1.
        configure(driver, {{"bReflectivity", false}, {"bDropNoise", false}, {"vColorDefault", {0.2, 0.4, 0.6}}});
        Access::packet(driver, data);
        Access::publish(driver);
        cloud.get(points);
        assert(points.size() == 6); // Disabling the filter also retains noise and unknown attributes.
        assert((points[0].m_vC - Vector3f(0.2f, 0.4f, 0.6f)).norm() < 0.0001f);
    }

    void testFrameCapAndClear()
    {
        DriverWithTransform driver;
        PCLframe cloud;
        Access::output(driver, cloud);
        configure(driver, {{"nMaxFramePoints", 96}});
        Access::packet(driver, packet(0, 1000000, 96));
        Access::packet(driver, packet(1, 1000100, 96));
        assert(Access::pending(driver) == 96);
        Access::packet(driver, packet(0, 1100000));
        vector<GEOMETRY_POINT> points;
        const uint64_t first = cloud.get(points);
        assert(points.size() == 96 && Access::status(driver).at("frameOverflows") == 1);
        assert(Access::status(driver).at("partialFrames") == 1);
        driver.clear();
        assert(Access::pending(driver) == 0 && cloud.get(points) > first && points.empty());
        Access::packet(driver, packet(0, 100)); // A new epoch still has increasing frame IDs.
        Access::publish(driver);
        assert(cloud.get(points) > first && points.size() == 1);
        assert(points[0].m_tStamp == 100000); // Point time retains sensor clock.
        driver.stop();
        driver.stop();
        assert(Access::pending(driver) == 0 && Access::status(driver).at("state") == "stopped");
    }

    void testClockAndOverflow()
    {
        DriverWithTransform driver;
        PCLframe cloud;
        Access::output(driver, cloud);
        Access::packet(driver, packet(0, 3000000));
        Access::packet(driver, packet(1, 3000100));
        Access::packet(driver, packet(0, 1000000)); // Sensor clock restarted.
        assert(Access::status(driver).at("clockResets") == 1);
        assert(Access::status(driver).at("partialFrames") == 1);
        auto changedTimeMode = packet(0, 2000000);
        changedTimeMode.timeMode = 3;
        Access::packet(driver, changedTimeMode);
        assert(Access::status(driver).at("clockResets") == 2);
        Access::publish(driver);
        configure(driver, {{"bUseSensorTime", false}});
        Access::packet(driver, packet(0, 0, 2), 5000000000ULL);
        Access::publish(driver);
        vector<GEOMETRY_POINT> points;
        assert(cloud.get(points) == 5000000000ULL && points.size() == 2);
        assert(points[0].m_tStamp == 5000000000ULL && points[1].m_tStamp == 5000001000ULL);
        configure(driver, {{"bUseSensorTime", true}});
        Access::packet(driver, packet(0, UINT64_MAX / 1000 + 1));
        assert(Access::pending(driver) == 0);
        assert(Access::status(driver).at("invalidPackets") == 1);
        configure(driver, {{"bUseSensorTime", false}});
        Access::packet(driver, packet(0, 1, 2), UINT64_MAX - 10);
        assert(Access::pending(driver) == 0);
        assert(Access::status(driver).at("invalidPackets") == 2);
    }

    void testConfigAtomicityAndTelemetry()
    {
        DriverWithTransform driver;
        json errors;
        const json before = Access::config(driver);
        Access::packet(driver, packet(0, 1000000));
        const json poseFields = {{"vPos", {1, 2, 3}}, {"vOrt", {0, 0, 0, 1}},
                                 {"vT", {1, 2, 3}}, {"vR", {0, 0, 0, 1}}};
        for (const auto &field : poseFields.items())
        {
            assert(!before.contains(field.key()) && !Access::schema(driver).contains(field.key()));
            assert(!Access::config(driver, {{field.key(), field.value()}}, errors));
            assert(errors.at(field.key()) == "Unknown parameter" && Access::config(driver) == before);
            assert(Access::pending(driver) == 1);
        }
        const json invalid[] = {
            json(), json::array(),
            {{"msopPort", 12345}, {"vRangeD", {10.0, 1.0}}},
            {{"msopPort", 7788}}, {{"difopPort", -1}}, {{"msopPort", 1.5}},
            {{"deviceIP", "192.168.1.999"}}, {{"bindIP", false}},
            {{"bDropNoise", "true"}}, {{"nMaxFramePoints", 95}},
            {{"vColorDefault", {0, 0, 1.1}}}, {{"vRangeD", {0, std::numeric_limits<double>::infinity()}}},
            {{"unknown", 1}}, {{"devicePHYmode", 2}},
        };
        for (const auto &patch : invalid)
        {
            assert(!Access::config(driver, patch, errors));
            assert(!errors.empty() && Access::config(driver) == before);
            assert(Access::pending(driver) == 1); // Failed config cannot drop an in-flight frame.
        }
        configure(driver, {{"vRangeD", {0.0, 20.0}}});
        assert(Access::pending(driver) == 0); // Successful changes start a coherent new frame.
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
    testFrames();
    testSinglePacketFrames();
    testTimeoutAndPartialPrefix();
    testFilteringAndTransform();
    testFrameCapAndClear();
    testClockAndOverflow();
    testConfigAtomicityAndTelemetry();
    std::cout << "E1R driver tests passed\n";
}
#endif
