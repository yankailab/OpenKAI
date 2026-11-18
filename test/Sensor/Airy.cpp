#ifdef OPENKAI_AIRY_TEST
#include "../../src/Sensor/LiDAR/RoboSense/Airy/_Airy.h"
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <thread>

namespace kai
{
    struct AiryTestAccess
    {
        static void output(_Airy &driver, PCLstream &cloud, IMUstream *imu = nullptr)
        { driver.m_pPCLout = &cloud; driver.m_pIMUout = imu; }
        static void packet(_Airy &driver, const airy::Msop &packet, uint64_t received = 100000000000ULL)
        { driver.handleMSOP(packet, received); }
        static void info(_Airy &driver, const airy::Difop &packet) { driver.handleDIFOP(packet, getTns()); }
        static void imu(_Airy &driver, const airy::Imu &packet, uint64_t received = 100000000000ULL)
        { driver.handleIMU(packet, received); }
        static bool config(_Airy &driver, const json &patch, json &errors)
        { return driver.applyConfig(patch, true, errors); }
        static json config(_Airy &driver) { return driver.configValues(); }
        static json schema(_Airy &driver) { return driver.controlSchema(); }
        static json status(_Airy &driver) { return driver.statusValues(); }
        static json imuValues(_Airy &driver) { return driver.imuValues(); }
        static bool previewEnabled(_Airy &driver) { return driver.m_bIMUpreview; }
        static bool levelingReady(_Airy &driver) { return driver.m_haveIMUleveling; }
        static uint64_t previewStamp(_Airy &driver) { return driver.m_previewStamp; }
        static void running(_Airy &driver, bool running) { driver.m_started = running; }
        static void resetSensor(_Airy &driver) { driver.resetSensor(true); }
        static json transport(_Airy &driver)
        {
            std::lock_guard<std::recursive_mutex> lock(driver.m_mutex);
            return {{"closed", driver.m_msopSocket < 0 && driver.m_difopSocket < 0 && driver.m_imuSocket < 0},
                {"ready", driver.m_msopSocket >= 0 && driver.m_difopSocket >= 0 && driver.m_imuSocket >= 0},
                {"error", driver.m_lastError}, {"invalidPackets", driver.m_invalidPackets}};
        }
    };
}

namespace
{
    using namespace kai;
    using Access = AiryTestAccess;

    class DriverWithTransform : public _Airy
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
        // Block the last socket so each failed attempt must release both earlier binds.
        UdpEndpoint msop, difop, imu;
        const uint16_t msopPort = msop.port(), difopPort = difop.port(), imuPort = imu.port();
        JsonCfg config;
        config.setJson({{"name", "airy-retry-test"}, {"class", "_Airy"},
            {"deviceIP", "127.0.0.1"}, {"bindIP", "127.0.0.1"},
            {"msopPort", msopPort}, {"difopPort", difopPort}, {"imuPort", imuPort}});
        PCLstream cloud;
        DriverWithTransform driver;
        driver.setConfig(&config, config.getJson());
        assert(driver.loadConfig());
        Access::output(driver, cloud);
        msop.close(); difop.close();
        assert(driver.start());
        await([&] { return !Access::transport(driver).at("error").get<string>().empty(); });
        assert(Access::transport(driver).at("closed") == true);
        {
            UdpEndpoint releasedMSOP(msopPort), releasedDIFOP(difopPort);
        }
        imu.close();
        await([&] { return Access::transport(driver).at("ready") == true; });
        assert(Access::transport(driver).at("error") == "");
        UdpEndpoint sender;
        sender.send(msopPort);
        await([&] { return Access::transport(driver).at("invalidPackets") == 1; });
        driver.stop();
        assert(Access::transport(driver).at("closed") == true);

        {
            UdpEndpoint blockedIMU(imuPort);
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
        UdpEndpoint releasedMSOP(msopPort), releasedDIFOP(difopPort), releasedIMU(imuPort);
    }

    const std::array<uint16_t, 8> early{{100, 200, 300, 400, 500, 600, 700, 800}};
    const std::array<uint16_t, 8> late{{20000, 22000, 24000, 26000, 28000, 30000, 32000, 34000}};
    const std::array<uint16_t, 8> zero{{0, 50, 100, 150, 200, 250, 300, 350}};

    airy::Msop packet(uint32_t sequence, uint64_t timestampNs,
                      std::array<uint16_t, 8> angles = early, size_t count = airy::kPointCount)
    {
        airy::Msop result;
        result.sequence = sequence;
        result.timestampNs = timestampNs;
        result.sequenceValid = true;
        result.model = 2;
        result.returnMode = 0;
        result.azimuths = angles;
        for (size_t i = 0; i < result.points.size(); ++i)
        {
            auto &point = result.points[i];
            point.block = static_cast<uint8_t>(i / 48);
            point.blockAzimuth = angles[point.block];
            point.offsetNs = static_cast<uint32_t>(point.block * 10000 + i % 48 * 100);
            point.timestampNs = timestampNs + point.offsetNs;
            point.x = point.range = i < count ? 1.0f : 0.0f;
            point.intensity = 128;
        }
        return result;
    }

    airy::Difop calibration()
    {
        airy::Difop result;
        result.calibration.valid = true;
        result.calibration.installMode = 0;
        result.calibration.vertical[0] = 100;
        result.rpm = 600;
        result.sensorIp = "192.168.1.200";
        result.destinationIp = "192.168.1.102";
        return result;
    }

    void configure(_Airy &driver, const json &config)
    {
        json errors;
        assert(Access::config(driver, config, errors));
        assert(errors.empty());
    }

    void testCalibration()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        Access::output(driver, cloud);
        vector<GEOMETRY_POINT> points;
        Access::packet(driver, packet(10, 1000000000));
        cloud.get(points, 0);
        assert(points.empty());
        assert(Access::status(driver).at("uncalibratedPackets") == 1);
        const auto info = calibration();
        Access::info(driver, info);
        assert(Access::status(driver).at("calibrationReady") == true);
        Access::packet(driver, packet(10, 1000000000));
        const uint64_t update = cloud.get(points, 0);
        assert(points.size() == 384);
        Access::info(driver, info);
        assert(cloud.get(points, 0) == update && points.size() == 384);
        auto changed = info;
        changed.calibration.horizontal[1] = 200;
        Access::info(driver, changed); // Stored points survive a calibration update.
        assert(cloud.get(points, 0) == update && points.size() == 384);
        Access::packet(driver, packet(11, 1001000000));
        cloud.get(points, 0);
        assert(points.size() == 768);
        changed.calibration.valid = false;
        Access::info(driver, changed); // Do not keep using a stale table.
        assert(!Access::status(driver).at("calibrationReady").get<bool>());
        Access::packet(driver, packet(12, 1002000000));
        cloud.get(points, 0);
        assert(points.size() == 768 && Access::status(driver).at("uncalibratedPackets") == 2);
    }

    void testImmediatePacketsAndDualReturns()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        Access::output(driver, cloud);
        Access::info(driver, calibration());
        const uint64_t before = getTns();
        Access::packet(driver, packet(10, 1000000000, {{35900, 35950, 0, 0, 50, 50, 100, 100}}));
        vector<GEOMETRY_POINT> points;
        const uint64_t update = cloud.get(points, 0);
        assert(update >= before && update <= getTns());
        assert(points.size() == 384); // A full packet is available across azimuth zero.
        assert(points.front().m_tStamp == 1000000000);
        assert(points.back().m_tStamp == 1000074700);
        Access::packet(driver, packet(11, 1001000000, late));
        Access::packet(driver, packet(12, 1002000000, zero));
        cloud.get(points, 0);
        assert(points.size() == 1152);
        cloud.get(points, 1001000000);
        assert(points.size() == 767 && points.front().m_tStamp == 1001000100);
        const json status = Access::status(driver);
        assert(status.at("pointPackets") == 3 && status.at("points") == 1152);
        assert(!status.contains("frames") && !status.contains("partialFrames") && !status.contains("frameOverflows"));

        _Airy dual;
        cloud.clear();
        Access::output(dual, cloud);
        Access::info(dual, calibration());
        auto returns = packet(1, 2000000000, {{35900, 35900, 35900, 35900, 0, 0, 0, 0}});
        returns.dual = true;
        Access::packet(dual, returns);
        cloud.get(points, 0);
        assert(points.size() == 384);
        assert(Access::status(dual).at("observed").at("dualReturn") == true);
    }

    void testSequenceAndRecovery()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        Access::output(driver, cloud);
        Access::info(driver, calibration());
        Access::packet(driver, packet(65535, 1000000000));
        Access::packet(driver, packet(0, 1001000000));
        Access::packet(driver, packet(0, 1001000000)); // Duplicate diagnostics do not filter points.
        Access::packet(driver, packet(65535, 1000000000)); // Reordered capture times are retained.
        Access::packet(driver, packet(3, 1003000000)); // 1,2 lost.
        vector<GEOMETRY_POINT> points;
        cloud.get(points, 0);
        assert(points.size() == 5 * 384);
        assert(points[3 * 384].m_tStamp == 1000000000);
        assert(Access::status(driver).at("outOfOrderPackets") == 2);
        assert(Access::status(driver).at("droppedPackets") == 2);
        Access::packet(driver, packet(4, 1004000000));
        assert(Access::status(driver).at("droppedPackets") == 2);
        // A prolonged MSOP pause must recover even while DIFOP/IMU remain alive.
        Access::packet(driver, packet(40000, 5000000000), 104000000000ULL);
        assert(Access::status(driver).at("outOfOrderPackets") == 2);
        Access::packet(driver, packet(0, 1000000000), 104000001000ULL);
        assert(Access::status(driver).at("clockResets") == 1);
        cloud.get(points, 0);
        assert(points.size() == 8 * 384 && points.back().m_tStamp == 1000074700);
    }

    void testCroppedFOV()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        Access::output(driver, cloud);
        auto info = calibration();
        info.calibration.fovStart = 10000;
        info.calibration.fovEnd = 12000;
        Access::info(driver, info);
        const std::array<uint16_t, 8> sector{{10000, 10300, 10600, 10900, 11200, 11500, 11800, 12000}};
        Access::packet(driver, packet(1, 1000000000, sector));
        vector<GEOMETRY_POINT> points;
        cloud.get(points, 0);
        assert(points.size() == 384);
        Access::packet(driver, packet(2, 1100000000, sector));
        cloud.get(points, 0);
        assert(points.size() == 768); // No revolution boundary is needed for output.
        assert(points[384].m_tStamp == 1100000000);
    }

    void testFilteringTransformAndCapacity()
    {
        DriverWithTransform driver;
        InspectableStream cloud;
        Access::output(driver, cloud);
        Access::info(driver, calibration());
        configure(driver, {{"vRangeD", {1.0, 10.0}}});
        driver.m_transform.setTranslation(1, 2, 3);
        driver.m_transform.setRotation(0, 0, 0.5, 0.5);
        auto data = packet(1, 1000000000, early, 5);
        data.points[0].x = data.points[0].range = 0.5f;
        data.points[2].x = data.points[2].range = 10.0f;
        data.points[3].x = data.points[3].range = 11.0f;
        Access::packet(driver, data);
        vector<GEOMETRY_POINT> points;
        cloud.get(points, 0);
        assert(points.size() == 3);
        assert((points[0].m_vP - Vector3f(1, 3, 3)).norm() < 0.0001f);
        assert((points[1].m_vP - Vector3f(1, 12, 3)).norm() < 0.0001f);
        assert((points[0].m_vC - Vector3f::Constant(128.0f / 255)).norm() < 0.0001f);
        assert(points[0].m_tStamp == 1000000100);
        configure(driver, {{"bReflectivity", false}, {"vColorDefault", {0.2, 0.4, 0.6}}});
        assert(cloud.clear(96)); // Capacity belongs to the output stream.
        Access::packet(driver, packet(2, 1001000000));
        cloud.get(points, 0);
        assert(points.size() == 96 && points.front().m_tStamp == 1001060000);
        assert(points.back().m_tStamp == 1001074700);
        assert(Access::status(driver).at("points") == 387); // Every filtered point was forwarded.
        assert((points[0].m_vC - Vector3f(.2f, .4f, .6f)).norm() < 0.0001f);
        driver.clear();
        cloud.get(points, 0);
        assert(points.empty());
        Access::packet(driver, packet(3, UINT64_MAX - 10));
        cloud.get(points, 0);
        assert(Access::status(driver).at("invalidPackets") == 1 && points.empty());
        configure(driver, {{"bUseSensorTime", false}});
        Access::packet(driver, packet(3, 1002000000), UINT64_MAX - 10);
        cloud.get(points, 0);
        assert(Access::status(driver).at("invalidPackets") == 2 && points.empty());
        Access::packet(driver, packet(3, 1002000000, early, 2), 5000000000ULL);
        cloud.get(points, 0);
        assert(points.size() == 2 && points[0].m_tStamp == 5000000000ULL && points[1].m_tStamp == 5000000100ULL);
        configure(driver, {{"bUseSensorTime", true}});
        Access::packet(driver, packet(4, 0, early, 1));
        assert(cloud.newest().m_tStamp == 0 && cloud.newest().m_vP.norm() > 0);
        cloud.get(points, 0);
        assert(points.size() == 2); // The reader chooses to omit invalid timestamps.
        driver.stop(); driver.stop();
        assert(Access::status(driver).at("state") == "stopped");
    }

    void testIMU()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        IMUstream imu;
        Access::output(driver, cloud, &imu);
        airy::Imu data;
        data.timestampNs = 5000000000;
        data.angularVelocity = {{.1, -.2, .3}};
        data.acceleration = {{1, 2, -9.80665}};
        Access::imu(driver, data);
        deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc);
        assert(gyro.size() == 1 && acc.size() == 1 && gyro[0].m_t == data.timestampNs);
        assert((gyro[0].m_v - Vector3f(.1f, -.2f, .3f)).norm() < 1e-6f);
        assert((acc[0].m_v - Vector3f(1, 2, -9.80665f)).norm() < 1e-6f);
        Access::imu(driver, data);
        assert(Access::status(driver).at("imuOutOfOrder") == 1);
        data.timestampNs = 1000000000; Access::imu(driver, data);
        imu.get(gyro, acc);
        assert(gyro.size() == 1 && gyro[0].m_t == 1000000000);
        assert(Access::status(driver).at("clockResets") == 1);
        configure(driver, {{"bIMUoutput", false}});
        data.timestampNs += 1000000; Access::imu(driver, data);
        imu.get(gyro, acc); assert(gyro.empty() && acc.empty());
        assert(Access::status(driver).at("imuPackets") == 4 && Access::status(driver).at("imuSamples") == 2);
        configure(driver, {{"bIMUoutput", true}, {"bUseSensorTime", false}});
        Access::imu(driver, data, 7000000000);
        imu.get(gyro, acc); assert(gyro.size() == 1 && gyro[0].m_t == 7000000000);
        driver.m_transform.setRotation(1, 0, 0, 0);
        Access::imu(driver, data, 7001000000);
        imu.get(gyro, acc); assert(acc.back().m_v.z() < 0); // Cloud mount does not silently rotate raw IMU.
        Access::resetSensor(driver);
        imu.get(gyro, acc); assert(gyro.empty() && acc.empty());
        assert(!Access::status(driver).at("calibrationReady").get<bool>());
        assert(Access::imuValues(driver).at("timestampNs") == "0");
    }

    void testIMUpreview()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        IMUstream imu;
        Access::output(driver, cloud, &imu);
        Access::running(driver, true);
        configure(driver, {{"bIMUpreview", true}});
        driver.m_transform.setTranslation(1, 2, 3);
        driver.m_transform.setRotation(0, 0, 0.5, 0.5);
        const auto mount = driver.m_transform.getPose().matrix().eval();
        airy::Imu data;
        data.timestampNs = 1000000000;
        data.acceleration = {{0, 0, -9.80665}};
        const auto sample = [&] {
            Access::imu(driver, data, getTns());
            return Access::imuValues(driver);
        };
        const auto quaternion = [](const json &values) {
            const auto &q = values.at("quaternion");
            return Quaterniond(q[0].get<double>(), q[1].get<double>(), q[2].get<double>(), q[3].get<double>());
        };
        json values = sample();
        assert(values.at("available") == true && values.at("orientationValid") == false);
        data.timestampNs += 5000000;
        values = sample();
        assert(values.at("fusion") == true && values.at("orientationValid") == true);
        const Quaterniond initial = quaternion(values);
        assert(std::abs(initial.norm() - 1) < 1e-10);
        assert((initial * Vector3d(0, 0, -1) - Vector3d::UnitZ()).norm() < 1e-6);
        assert(values.at("tGyro") == std::to_string(data.timestampNs));
        assert(values.at("tAcc") == values.at("tGyro") && values.at("tFusion") == values.at("tGyro"));
        assert(values.at("acc")[2] == -9.80665); // Graphs retain raw sensor axes.

        // Integrate one second of known angular velocity using capture times,
        // regardless of how quickly the test/browser reads the preview.
        data.angularVelocity = {{0, 0, 1}};
        for (unsigned i = 0; i < 200; ++i) { data.timestampNs += 5000000; values = sample(); }
        const Quaterniond after = quaternion(values);
        const Quaterniond relative = initial.conjugate() * after;
        assert(std::abs(2 * std::atan2(relative.z(), relative.w()) - 1.0) < 0.01);
        assert((driver.m_transform.getPose().matrix() - mount).norm() == 0); // No point cloud stabilization.
        deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc);
        assert(gyro.size() == 202 && acc.size() == 202);
        assert((gyro.back().m_v - Vector3f(0, 0, 1)).norm() == 0 && acc.back().m_v.z() < 0);
        const auto samples = Access::status(driver).at("imuSamples");

        configure(driver, {{"bIMUoutput", false}});
        assert(Access::imuValues(driver).at("orientationValid") == true);
        assert(Access::imuValues(driver).at("tGyro") == std::to_string(data.timestampNs));
        for (unsigned i = 0; i < 20; ++i) { data.timestampNs += 5000000; values = sample(); }
        assert(values.at("enabled") == false && values.at("available") == true && values.at("orientationValid") == true);
        assert(std::abs((after.conjugate() * quaternion(values)).angularDistance(Quaterniond::Identity()) - 0.1) < 0.01);
        imu.get(gyro, acc); assert(gyro.empty() && acc.empty());
        assert(Access::status(driver).at("imuSamples") == samples);

        // Reconnects, clock resets and long gaps reseed gravity without a giant
        // gyro integration step. Zero acceleration cannot claim a valid pose.
        data.timestampNs += 600000000; values = sample();
        assert(values.at("orientationValid") == false);
        data.timestampNs += 5000000; values = sample();
        assert(values.at("orientationValid") == true);
        data.timestampNs = 1000000000; values = sample();
        assert(values.at("orientationValid") == false);
        data.timestampNs += 5000000; values = sample();
        assert(values.at("orientationValid") == true);
        data.acceleration = {{0, 0, 0}}; data.timestampNs += 5000000; values = sample();
        assert(values.at("orientationValid") == false);
        driver.clear();
        values = Access::imuValues(driver);
        assert(values.at("tGyro") == "0" && values.at("tFusion") == "0" && values.at("orientationValid") == false);
        Access::resetSensor(driver);
        assert(Access::imuValues(driver).at("available") == false);
        driver.stop();
    }

    void testPreviewOptIn()
    {
        DriverWithTransform driver;
        PCLstream cloud;
        IMUstream imu;
        Access::output(driver, cloud, &imu);
        Access::running(driver, true);
        assert(!Access::previewEnabled(driver));
        assert(Access::config(driver).at("bIMUpreview") == false);
        assert(Access::schema(driver).at("bIMUpreview").at("type") == "bool");
        Access::info(driver, calibration());
        Access::packet(driver, packet(1, 1000000000));
        vector<GEOMETRY_POINT> points;
        const uint64_t cloudUpdate = cloud.get(points, 0);
        const size_t storedPoints = points.size();
        airy::Imu data;
        data.timestampNs = 1000000000;
        data.acceleration = {{0, 0, -9.80665}};
        data.angularVelocity = {{0, 0, 1}};
        const auto sample = [&] {
            data.timestampNs += 5000000;
            Access::imu(driver, data, getTns());
            return Access::imuValues(driver);
        };
        json values;
        for (unsigned i = 0; i < 20; ++i) values = sample();
        assert(values.at("previewEnabled") == false && values.at("available") == false);
        assert(values.at("rawAvailable") == true && values.at("deviceOpen") == true);
        assert(values.at("fusion") == false && values.at("orientationValid") == false && values.at("tFusion") == "0");
        assert(values.at("gyro")[2] == 1 && values.at("acc")[2] == -9.80665);
        assert(Access::previewStamp(driver) == 0 && !Access::levelingReady(driver));
        deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc);
        assert(gyro.size() == 20 && acc.size() == 20); // Disabled preview still supplies SLAM.

        configure(driver, {{"bIMUpreview", true}});
        assert(Access::previewEnabled(driver) && cloud.get(points, 0) == cloudUpdate && points.size() == storedPoints);
        imu.get(gyro, acc); assert(gyro.size() == 20); // Enabling does not consume raw history.
        assert(Access::imuValues(driver).at("tFusion") == "0");
        values = sample(); assert(values.at("orientationValid") == false);
        values = sample(); assert(values.at("orientationValid") == true);
        for (unsigned i = 0; i < 30; ++i) values = sample();
        const auto rawCount = Access::status(driver).at("imuSamples").get<size_t>();
        configure(driver, {{"bIMUpreview", false}});
        assert(!Access::previewEnabled(driver) && cloud.get(points, 0) == cloudUpdate && points.size() == storedPoints);
        for (unsigned i = 0; i < 20; ++i) values = sample();
        assert(Access::status(driver).at("imuSamples") == rawCount + 20);
        assert(values.at("previewEnabled") == false && values.at("tFusion") == "0");
        assert(values.at("quaternion") == json({1.0, 0.0, 0.0, 0.0}));
        assert(Access::previewStamp(driver) == 0 && !Access::levelingReady(driver));
        assert(Access::status(driver).at("imu").at("available") == false);

        // Re-enabling starts a fresh gravity seed, never the previous integrated
        // yaw or the samples accumulated while fusion was disabled.
        configure(driver, {{"bIMUpreview", true}});
        assert(Access::imuValues(driver).at("orientationValid") == false);
        assert(Access::imuValues(driver).at("tFusion") == "0");
        data.acceleration = {{0, 0, 9.80665}};
        data.angularVelocity = {{0, 0, 0}};
        values = sample(); assert(values.at("orientationValid") == false);
        values = sample(); assert(values.at("orientationValid") == true);
        const auto &q = values.at("quaternion");
        assert(std::abs(q[0].get<double>() - 1) < 1e-6);
        for (unsigned i = 1; i < 4; ++i) assert(std::abs(q[i].get<double>()) < 1e-6);
        configure(driver, {{"bIMUpreview", false}});
        driver.clear();
        Access::resetSensor(driver);
        assert(Access::previewStamp(driver) == 0 && !Access::levelingReady(driver));
        driver.stop();
    }

    void testConfigPreservesLinkedStreams()
    {
        JsonCfg config;
        config.setJson({{"name", "airy-config-test"}, {"class", "_Airy"},
            {"deviceIP", "127.0.0.2"}, {"bUseSensorTime", false}, {"bIMUoutput", false}});
        PCLstream cloud;
        IMUstream imu;
        DriverWithTransform driver;
        driver.setConfig(&config, config.getJson());
        Access::output(driver, cloud, &imu);
        imu.addGyro(Vector3f(1, 2, 3), 100);
        imu.addAcc(Vector3f(4, 5, 6), 100);
        GEOMETRY_POINT point;
        point.m_vP = Vector3f(7, 8, 9);
        point.m_tStamp = 100;
        cloud.add({point}, 100);
        const auto assertStreamsUnchanged = [&]
        {
            deque<IMUstream::IMU_DATA> gyro, acc;
            assert(imu.get(gyro, acc) == 100 && gyro.size() == 1 && acc.size() == 1);
            assert(gyro[0].m_t == 100 && gyro[0].m_v.isApprox(Vector3f(1, 2, 3)));
            assert(acc[0].m_t == 100 && acc[0].m_v.isApprox(Vector3f(4, 5, 6)));
            vector<GEOMETRY_POINT> points;
            assert(cloud.get(points, 0) == 100 && points.size() == 1);
            assert(points[0].m_tStamp == 100 && points[0].m_vP.isApprox(point.m_vP));
        };
        assert(driver.loadConfig());
        assertStreamsUnchanged();
        assert(driver.saveConfig(false));
        assertStreamsUnchanged();
        config.getJson()->update({{"deviceIP", "127.0.0.3"}, {"bUseSensorTime", true}, {"bIMUoutput", true}});
        assert(driver.loadConfig());
        assertStreamsUnchanged();
        assert(Access::config(driver).at("deviceIP") == "127.0.0.3");
        // A live clock/output change still discards old samples deliberately.
        configure(driver, {{"bIMUoutput", false}});
        deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc);
        assert(gyro.empty() && acc.empty());
    }

    void testConfig()
    {
        DriverWithTransform driver;
        json errors;
        const json original = Access::config(driver);
        for (const auto &field : original.items()) assert(Access::schema(driver).contains(field.key()));
        configure(driver, original); // All defaults fit their documented limits.
        const json poseFields = {{"vPos", {1, 2, 3}}, {"vOrt", {0, 0, 0, 1}},
                                 {"vT", {1, 2, 3}}, {"vR", {0, 0, 0, 1}}};
        for (const auto &field : poseFields.items())
        {
            assert(!original.contains(field.key()) && !Access::schema(driver).contains(field.key()));
            assert(!Access::config(driver, {{field.key(), field.value()}}, errors));
            assert(errors.at(field.key()) == "Unknown parameter" && Access::config(driver) == original);
        }
        const json invalid[] = {json(), json::array(), {{"msopPort", 7788}}, {{"imuPort", 6699}},
            {{"imuPort", -1}}, {{"vRangeD", {0, 120}}}, {{"vRangeD", {10, 1}}}, {{"splitAngle", 360}},
            {{"nMaxFramePoints", 96}}, {{"frameTimeoutMs", 250}},
            {{"bIMUoutput", 1}}, {{"bIMUpreview", 1}}, {{"bIMUpreview", "true"}},
            {{"bIMUpreview", nullptr}}, {{"deviceIP", "999.0.0.1"}},
            {{"bReflectivity", false}, {"unexpected", 1}}, {{"deviceRPM", 1200}}};
        for (const auto &patch : invalid)
        {
            assert(!Access::config(driver, patch, errors));
            assert(!errors.empty() && Access::config(driver) == original);
        }
        configure(driver, {{"imuPort", 0}});
        assert(Access::config(driver).at("imuPort") == 0);
        Access::info(driver, calibration());
        assert(Access::config(driver).at("deviceRPM") == 600);
        assert(Access::config(driver).at("deviceSensorIP") == "192.168.1.200");
        configure(driver, Access::config(driver)); // Unchanged observed fields can round-trip.
        assert(!Access::config(driver, {{"deviceRPM", 1200}}, errors));
        driver.stop(); driver.stop();
        assert(Access::status(driver).at("state") == "stopped");
    }
}

int main()
{
    testSocketRetries();
    testCalibration();
    testImmediatePacketsAndDualReturns();
    testSequenceAndRecovery();
    testCroppedFOV();
    testFilteringTransformAndCapacity();
    testIMU();
    testIMUpreview();
    testPreviewOptIn();
    testConfigPreservesLinkedStreams();
    testConfig();
    std::cout << "Airy driver tests passed\n";
}
#endif
