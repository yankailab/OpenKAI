#ifdef OPENKAI_REALSENSE_PREVIEW_TEST
#include "../../src/Vision/RGBD/_RealSense.h"
#include <librealsense2/hpp/rs_internal.hpp>
#include <cassert>
#include <iostream>
#include <thread>

namespace kai
{
    struct RealSensePreviewTestAccess
    {
        static void output(_RealSense &driver, IMUstream &imu) { driver.m_pIMUout = &imu; }
        static void opened(_RealSense &driver, bool value) { driver.m_bOpened = value; }
        static void frame(_RealSense &driver, rs2::frame frame) { driver.receiveFrame(std::move(frame)); }
        static bool config(_RealSense &driver, const json &patch, json &errors)
        { return driver.applyConfig(patch, true, errors); }
        static json config(_RealSense &driver) { return driver.configValues(); }
        static json schema(_RealSense &driver) { return driver.controlSchema(); }
        static json values(_RealSense &driver)
        {
            std::lock_guard<std::recursive_mutex> lock(driver.m_deviceMutex);
            return driver.imuValues();
        }
        // Inspect retained state to detect accidental reset/add calls while off.
        static IMUpreview::Snapshot retained(_RealSense &driver)
        {
            std::lock_guard<std::mutex> lock(driver.m_imuPreviewMutex);
            return driver.m_imuPreview.snapshot();
        }
    };
}

namespace
{
    using namespace kai;
    using Access = RealSensePreviewTestAccess;

    void configure(_RealSense &driver, const json &patch)
    {
        json errors;
        const bool ok = Access::config(driver, patch, errors);
        if (!ok) std::cerr << errors.dump() << '\n';
        assert(ok);
    }

    class MotionSource
    {
    public:
        explicit MotionSource(_RealSense &driver)
            : m_sensor(m_device.add_sensor("OpenKAI motion test"))
        {
            rs2_motion_stream stream{};
            stream.type = RS2_STREAM_ACCEL; stream.uid = 1; stream.fps = 200;
            stream.fmt = RS2_FORMAT_MOTION_XYZ32F;
            m_acc = m_sensor.add_motion_stream(stream);
            stream.type = RS2_STREAM_GYRO; stream.uid = 2;
            m_gyro = m_sensor.add_motion_stream(stream);
            m_sensor.open(std::vector<rs2::stream_profile>{m_acc, m_gyro});
            m_sensor.start([&driver](rs2::frame frame) { Access::frame(driver, std::move(frame)); });
        }

        ~MotionSource() { m_sensor.stop(); m_sensor.close(); }

        void sample()
        {
            send(m_acc, {0, 0, 9.80665f});
            send(m_gyro, {0, 0, 0.3f});
            m_timeMs += 5;
            ++m_number;
        }

    private:
        void send(const rs2::stream_profile &profile, rs2_vector value)
        {
            rs2_software_motion_frame frame{};
            frame.data = new rs2_vector(value);
            frame.deleter = [](void *data) { delete static_cast<rs2_vector *>(data); };
            frame.timestamp = m_timeMs;
            frame.domain = RS2_TIMESTAMP_DOMAIN_HARDWARE_CLOCK;
            frame.frame_number = m_number;
            frame.profile = profile.get();
            m_sensor.on_motion_frame(frame);
        }

        rs2::software_device m_device;
        rs2::software_sensor m_sensor;
        rs2::stream_profile m_acc, m_gyro;
        double m_timeMs = 1000;
        int m_number = 1;
    };

    void run()
    {
        JsonCfg config;
        config.setJson({{"name", "realsense-preview-test"}, {"class", "_RealSense"},
            {"bRGB", false}, {"bDepth", false}, {"bIR", false}, {"bIMU", true},
            {"bPCL", false}, {"bPCLrgb", false}, {"bAlign", false}});
        IMUstream imu;
        _RealSense driver;
        driver.BASE::setConfig(&config, config.getJson());
        assert(driver.loadConfig());
        Access::output(driver, imu);
        assert(Access::config(driver).at("bIMUpreview") == false);
        const auto schema = Access::schema(driver);
        const auto field = std::find_if(schema.begin(), schema.end(), [](const json &entry) {
            return entry.at("key") == "bIMUpreview";
        });
        assert(field != schema.end() && field->at("type") == "bool" && field->at("restart") == false);
        json errors;
        assert(!Access::config(driver, {{"bIMUpreview", 1}}, errors));
        assert(errors.contains("bIMUpreview") && Access::config(driver).at("bIMUpreview") == false);

        // Mark the module open without a hardware pipeline. Preview-only edits
        // must succeed without trying to reopen a physical camera.
        Access::opened(driver, true);
        MotionSource source(driver);
        source.sample(); source.sample();
        auto values = Access::values(driver);
        assert(values.at("enabled") == true && values.at("previewEnabled") == false);
        assert(values.at("available") == false && values.at("tGyro") == "0");
        assert(Access::retained(driver).tGyro == 0);

        configure(driver, {{"bIMUpreview", true}});
        assert(Access::values(driver).at("available") == false);
        source.sample(); source.sample();
        values = Access::values(driver);
        assert(values.at("deviceOpen") == true && values.at("previewEnabled") == true);
        assert(values.at("available") == true && values.at("orientationValid") == true);
        const auto retained = Access::retained(driver);
        configure(driver, {{"bIMUpreview", false}});
        source.sample(); source.sample();
        values = Access::values(driver);
        assert(values.at("available") == false && values.at("orientationValid") == false);
        assert(values.at("fusion") == false && values.at("tFusion") == "0");
        assert(Access::retained(driver).tGyro == retained.tGyro);
        assert(Access::retained(driver).tAcc == retained.tAcc);
        driver.close();
        assert(Access::retained(driver).tFusion == retained.tFusion); // No reset while off.

        configure(driver, {{"bIMUpreview", true}});
        assert(Access::retained(driver).tGyro == 0 && Access::retained(driver).tFusion == 0);
        Access::opened(driver, true);
        source.sample();
        assert(Access::values(driver).at("orientationValid") == false);
        source.sample();
        assert(Access::values(driver).at("orientationValid") == true);

        // SDK callbacks must not race preview toggles or wait for the device
        // mutex used by close()/pipeline.stop(). Raw samples are never gated.
        std::thread producer([&source] { for (int i = 0; i < 100; ++i) source.sample(); });
        for (int i = 0; i < 100; ++i) configure(driver, {{"bIMUpreview", i % 2 == 0}});
        producer.join();
        std::deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc);
        assert(gyro.size() == 108 && acc.size() == 108);
        assert(std::abs(acc.back().m_v.z() - 9.80665f) < 1e-5f);
        assert(std::abs(gyro.back().m_v.z() - 0.3f) < 1e-6f);

        configure(driver, {{"bIMUpreview", true}});
        assert(driver.saveConfig(false));
        assert(config.getJson()->at("bIMUpreview") == true);
        Access::opened(driver, false);
        _RealSense restored;
        restored.BASE::setConfig(&config, config.getJson());
        assert(restored.loadConfig() && Access::config(restored).at("bIMUpreview") == true);
        configure(restored, {{"bIMUpreview", false}});
        assert(restored.saveConfig(false) && config.getJson()->at("bIMUpreview") == false);
    }
}

int main()
{
    run();
    std::cout << "RealSense IMU preview switch tests passed\n";
}
#endif
