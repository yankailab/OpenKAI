#include "../../src/Vision/RGBD/_Orbbec.h"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>
#include <iostream>
using namespace kai;

class Replies : public _JSONbase
{
public:
    std::mutex mutex;
    std::vector<json> messages;
    std::atomic<bool> holdIMU{false}, sendingIMU{false};
    bool sendJson(const json &j) override
    {
        if (j.value("cmd", "") == "imuData" && holdIMU)
        {
            sendingIMU = true;
            while (holdIMU) std::this_thread::sleep_for(std::chrono::milliseconds(1));
            sendingIMU = false;
        }
        std::lock_guard<std::mutex> lock(mutex); messages.push_back(j); return true;
    }
    json last() { std::lock_guard<std::mutex> lock(mutex); assert(!messages.empty()); return messages.back(); }
    size_t count() { std::lock_guard<std::mutex> lock(mutex); return messages.size(); }
};

class TestIMU : public _IMUbase
{
public:
    using _IMUbase::m_mtxIMU;
    using _IMUbase::m_mtxPreview;
    using _IMUbase::m_dqFusionGyro;
    using _IMUbase::m_dqFusionAcc;
    using _IMUbase::m_tFusionPreview;
    using _IMUbase::m_vAnglePreview;
    using _IMUbase::m_pTstream;
};

int main(int argc, char **argv)
{
    assert(argc == 2);
    const std::string path = std::string(argv[1]) + "/controls.json";
    const json config = {{"class", "_Orbbec"}, {"thread", {{"FPS", 200}}}, {"threadPP", {{"FPS", 30}}}, {"fConfig", path}};
    Replies replies;
    _Orbbec camera;
    camera.setName("Orbbec"); assert(camera.init(config));
    camera.console(json{{"cmd", "loadConfig"}, {"requestId", 17}}, &replies);
    const json initial = replies.last();
    assert(initial["bSuccess"] && initial["requestId"] == 17);
    assert(initial["schema"].size() > 120);
    assert(initial["config"]["OB_PROP_COLOR_EXPOSURE_INT"].is_null());
    for (const auto &patch : {json{{"OB_PROP_COLOR_GAIN_INT", 2.5}}, json{{"OB_PROP_LASER_BOOL", 1}},
          json{{"OB_PROP_COLOR_GAIN_INT", 4294967296ULL}}, json{{"tOutMs", -1}}, json{{"SN", 22}},
          json{{"vSizeRGB", {640}}}, json{{"OB_STRUCT_DEPTH_HDR_CONFIG", {{"enable", true}}}}, json{{"unknown", 3}}})
    {
        camera.console(json{{"cmd", "setConfig"}, {"config", patch}}, &replies);
        assert(!replies.last()["bSuccess"].get<bool>());
        assert(replies.last()["config"] == initial["config"]);
    }
    camera.console(json{{"cmd", "setConfig"}, {"config", {{"OB_PROP_COLOR_GAIN_INT", 16}}}}, &replies);
    assert(!replies.last()["bSuccess"].get<bool>()); // Offline edit must not claim a device write.
    assert(!std::filesystem::exists(path));

    // Public setters cache requested overrides without writing a file.
    assert(!camera.setColorGain(16));
    assert(!camera.setLaser(false));
    OBRegionOfInterest roi{}; roi.x1_right = 639; roi.y1_bottom = 479;
    assert(!camera.setColorAEROI(roi));
    assert(!std::filesystem::exists(path));
    camera.console(json{{"cmd", "saveConfig"}}, &replies);
    assert(replies.last()["bSuccess"].get<bool>());
    json saved; std::ifstream(path) >> saved;
    assert(saved.size() == 3 && saved["OB_PROP_COLOR_GAIN_INT"] == 16 && saved["OB_PROP_LASER_BOOL"] == false);
    assert(saved["OB_STRUCT_COLOR_AE_ROI"]["x1_right"] == 639);
    {
        _Orbbec restored; restored.setName("Orbbec"); assert(restored.init(config));
        assert(restored.getCamCtrl().m_colorGain == 16);
        assert(restored.getCamCtrl().m_laser == false);
        assert(restored.getCamCtrl().m_colorAEROI->y1_bottom == 479);
        json again; assert(restored.saveConfig(again)); assert(again == saved);
        // Clearing a nullable override removes it from subsequent files.
        std::ofstream(path) << json{{"OB_PROP_COLOR_GAIN_INT", nullptr}};
        assert(restored.loadConfig()); assert(!restored.getCamCtrl().m_colorGain);
        assert(restored.saveConfig(again)); assert(!again.contains("OB_PROP_COLOR_GAIN_INT"));
        assert(!restored.saveConfig(again, std::string(argv[1]) + "/missing/file.json"));
    }
    std::cout << "PASS: Orbbec schema, malformed/offline commands, sparse save, null removal and reload" << std::endl;

    const json imuConfig = {{"class", "_IMUbase"}, {"thread", {{"FPS", 1}}}, {"threadStream", {{"FPS", 17}}}, {"bFusion", true}, {"tIMUpairToleranceUs", 5000}};
    TestIMU imu; imu.setName("obIMU"); assert(imu.init(imuConfig));
    assert(imu.m_pTstream->getTargetFPS() == 17);
    Vector3f g, a;
    imu.addGyro({1, 2, 3}, 10000); imu.addAcc({4, 5, 6}, 20001);
    assert(imu.getIMUpair(&g, &a) == 0);
    imu.addGyro({1, 2, 3}, 20000);
    assert(imu.getIMUpair(&g, &a) == 20001 && g.x() == 1 && a.z() == 6);
    // Integrate 1 rad/s for exactly 1 second. All timestamps are microseconds.
    for (int i = 0; i <= 200; ++i) { imu.addGyro({0, 0, 1}, 1000000 + i * 5000); imu.addAcc({0, 0, 9.81f}, 1000000 + i * 5000); }
    imu.console(json{{"cmd", "startStream"}}, &replies);
    assert(replies.last()["streaming"].get<bool>());
    assert(imu.start());
    json sample;
    for (int i = 0; i < 100; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); sample = replies.last();
        if (sample["cmd"] == "imuData" && sample["tFusion"] == 2000000) break;
    }
    assert(sample["cmd"] == "imuData" && sample["orientationValid"].get<bool>());
    assert(std::abs(sample["rpy"][2].get<double>() - 1.0) < 0.001);
    assert(std::abs(sample["quaternion"][0].get<double>() - std::cos(0.5)) < 0.001);
    assert(sample["gyro"][2] == 1 && sample["acc"][2].get<float>() == 9.81f);
    assert(imu.getIMUpair(&g, &a) == 1000000); // Fusion did not steal SLAM's samples.
    // Block the transport and preview reader independently of the fast worker.
    replies.holdIMU = true;
    imu.addGyro({0, 0, 1}, 2005000); imu.addAcc({0, 0, 9.81f}, 2005000);
    for (int i = 0; i < 100 && !replies.sendingIMU; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    assert(replies.sendingIMU);
    {
        std::lock_guard<std::mutex> reader(imu.m_mtxPreview);
        for (int i = 1; i <= 400; ++i)
        {
            imu.addGyro({0, 0, -1}, 2005000 + i * 5000);
            imu.addAcc({0, 0, 9.81f}, 2005000 + i * 5000);
        }
        bool drained = false;
        for (int i = 0; i < 100 && !drained; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            std::lock_guard<std::mutex> lock(imu.m_mtxIMU);
            drained = imu.m_dqFusionGyro.empty() && imu.m_dqFusionAcc.empty();
        }
        assert(drained); // No 1 Hz throttle, and neither preview nor send blocks fusion.
    }
    bool published = false;
    for (int i = 0; i < 100 && !published; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        std::lock_guard<std::mutex> lock(imu.m_mtxPreview);
        published = imu.m_tFusionPreview == 4005000;
        if (published)
        {
            assert(std::abs(imu.m_vAnglePreview.z() + 0.995) < 0.001);
            assert(std::abs(imu.m_vAnglePreview.x()) < 0.001);
            assert(std::abs(imu.m_vAnglePreview.y()) < 0.001);
        }
    }
    assert(published && replies.sendingIMU);
    replies.holdIMU = false;
    imu.console(json{{"cmd", "stopStream"}}, &replies);
    const size_t count = replies.count();
    imu.addGyro({0, 0, 1}, 4010000); imu.addAcc({0, 0, 9.81f}, 4010000);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); assert(replies.count() == count);
    imu.stop();
    std::cout << "PASS: IMU pairing, fusion units/quaternion, independent raw preview, start/stop and shutdown" << std::endl;
    std::cout << "PASS: configured preview FPS, negative-yaw Eigen angles, full-speed fusion during blocked preview/send" << std::endl;
}
