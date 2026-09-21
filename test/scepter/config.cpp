#include "../../src/Vision/RGBD/_Scepter.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace kai;

namespace {
std::vector<string> calls;
string reject;
int rejectedHDRindex = -1;
ScStatus record(const char *name) {
    calls.emplace_back(name);
    return reject == name ? SC_INVALID_DEVICE_INDEX : SC_OK;
}
}
// SDK setters are intercepted below so live commands can be checked without
// contacting a camera. The installed SDK still supplies types and other APIs.
extern "C" ScStatus scAIModuleSetEnabled(ScDeviceHandle device, bool bEnabled) { return record("scAIModuleSetEnabled"); }
extern "C" ScStatus scAIModuleSetInputFrameTypeEnabled(ScDeviceHandle device, ScFrameType frameType, bool bEnabled) { return record("scAIModuleSetInputFrameTypeEnabled"); }
extern "C" ScStatus scAIModuleSetParam(ScDeviceHandle device, uint32_t paramID, void* pBuffer, uint16_t bufferSize) { return record("scAIModuleSetParam"); }
extern "C" ScStatus scAIModuleSetPreviewFrameTypeEnabled(ScDeviceHandle device, ScFrameType frameType, bool bEnabled) { return record("scAIModuleSetPreviewFrameTypeEnabled"); }
extern "C" ScStatus scAIModuleSetWorkMode(ScDeviceHandle device, ScAIModuleMode mode) { return record("scAIModuleSetWorkMode"); }
extern "C" ScStatus scGetSensorIntrinsicParameters(ScDeviceHandle device, ScSensorType sensorType, ScSensorIntrinsicParameters* pSensorIntrinsicParameters) { return record("scGetSensorIntrinsicParameters"); }
extern "C" ScStatus scSetColorAECMaxExposureTime(ScDeviceHandle device, int32_t exposureTime) { return record("scSetColorAECMaxExposureTime"); }
extern "C" ScStatus scSetColorAECROI(ScDeviceHandle device, uint16_t x, uint16_t y, uint16_t width, uint16_t height) { return record("scSetColorAECROI"); }
extern "C" ScStatus scSetColorGain(ScDeviceHandle device, float params) { return record("scSetColorGain"); }
extern "C" ScStatus scSetColorPixelFormat(ScDeviceHandle device, ScPixelFormat pixelFormat) { return record("scSetColorPixelFormat"); }
extern "C" ScStatus scSetColorResolution(ScDeviceHandle device, int32_t w, int32_t h) { return record("scSetColorResolution"); }
extern "C" ScStatus scSetConfidenceFilterParams(ScDeviceHandle device, ScConfidenceFilterParams params) { return record("scSetConfidenceFilterParams"); }
extern "C" ScStatus scSetDeviceDHCPEnabled(ScDeviceHandle device, bool bEnabled) { return record("scSetDeviceDHCPEnabled"); }
extern "C" ScStatus scSetDeviceIPAddr(ScDeviceHandle device, const char* ipAddr, int32_t length) { return record("scSetDeviceIPAddr"); }
extern "C" ScStatus scSetDeviceSubnetMask(ScDeviceHandle device, const char* pMask, int32_t length) { return record("scSetDeviceSubnetMask"); }
extern "C" ScStatus scSetExposureControlMode(ScDeviceHandle device, ScSensorType sensorType, ScExposureControlMode controlMode) { return record("scSetExposureControlMode"); }
extern "C" ScStatus scSetExposureTime(ScDeviceHandle device, ScSensorType sensorType, int32_t exposureTime) { return record("scSetExposureTime"); }
extern "C" ScStatus scSetExposureTimeOfHDR(ScDeviceHandle device, uint8_t frameIndex, int32_t exposureTime) { calls.emplace_back("scSetExposureTimeOfHDR"); return frameIndex == rejectedHDRindex ? SC_INVALID_PARAMS : SC_OK; }
extern "C" ScStatus scSetExposureTimeOfWDR(ScDeviceHandle device, uint8_t frameIndex, int32_t exposureTime) { return record("scSetExposureTimeOfWDR"); }
extern "C" ScStatus scSetFillHoleFilterEnabled(ScDeviceHandle device, bool bEnabled) { return record("scSetFillHoleFilterEnabled"); }
extern "C" ScStatus scSetFlyingPixelFilterParams(ScDeviceHandle device, const ScFlyingPixelFilterParams params) { return record("scSetFlyingPixelFilterParams"); }
extern "C" ScStatus scSetFrameRate(ScDeviceHandle device, int32_t value) { return record("scSetFrameRate"); }
extern "C" ScStatus scSetHDRModeEnabled(ScDeviceHandle device, bool bEnabled) { return record("scSetHDRModeEnabled"); }
extern "C" ScStatus scSetHotPlugStatusCallback(PtrHotPlugStatusCallback pCallback, const void* pUserData) { return record("scSetHotPlugStatusCallback"); }
extern "C" ScStatus scSetIRGMMCorrection(ScDeviceHandle device, const ScIRGMMCorrectionParams params) { return record("scSetIRGMMCorrection"); }
extern "C" ScStatus scSetIRGMMGain(ScDeviceHandle device, uint8_t gmmgain) { return record("scSetIRGMMGain"); }
extern "C" ScStatus scSetInputSignalParamsForHWTrigger(ScDeviceHandle device, ScInputSignalParamsForHWTrigger params) { return record("scSetInputSignalParamsForHWTrigger"); }
extern "C" ScStatus scSetParamsByJson(ScDeviceHandle device, char* pfilePath) { return record("scSetParamsByJson"); }
extern "C" ScStatus scSetRealTimeSyncConfig(ScDeviceHandle device, ScTimeSyncConfig params) { return record("scSetRealTimeSyncConfig"); }
extern "C" ScStatus scSetSoftwareTriggerParameter(ScDeviceHandle device, uint8_t frameCount) { return record("scSetSoftwareTriggerParameter"); }
extern "C" ScStatus scSetSpatialFilterEnabled(ScDeviceHandle device, bool bEnabled) { return record("scSetSpatialFilterEnabled"); }
extern "C" ScStatus scSetTimeFilterParams(ScDeviceHandle device, ScTimeFilterParams params) { return record("scSetTimeFilterParams"); }
extern "C" ScStatus scSetToFResolution(ScDeviceHandle device, int32_t w, int32_t h) { return record("scSetToFResolution"); }
extern "C" ScStatus scSetTransformColorImgToDepthSensorEnabled(ScDeviceHandle device, bool bEnabled) { return record("scSetTransformColorImgToDepthSensorEnabled"); }
extern "C" ScStatus scSetTransformDepthImgToColorSensorEnabled(ScDeviceHandle device, bool bEnabled) { return record("scSetTransformDepthImgToColorSensorEnabled"); }
extern "C" ScStatus scSetWDRModeEnabled(ScDeviceHandle device, bool bEnabled) { return record("scSetWDRModeEnabled"); }
extern "C" ScStatus scSetWorkMode(ScDeviceHandle device, ScWorkMode mode) { return record("scSetWorkMode"); }
extern "C" ScStatus scStartStream(ScDeviceHandle device) { return record("scStartStream"); }
extern "C" ScStatus scStopStream(ScDeviceHandle device) { return record("scStopStream"); }

class Replies : public _JSONbase {
public:
    json last;
    bool sendJson(const json &j) override { last = j; return true; }
};
class Camera : public _Scepter {
public:
    void connected(bool on) { m_bOpened = on; m_scDevHandle = on ? this : nullptr; }
};

int main(int argc, char **argv) {
    assert(argc == 2);
    const string path = string(argv[1]) + "/controls.json";
    const json startup = {{"class", "_Scepter"}, {"thread", {{"FPS", 30}}}, {"threadPP", {{"FPS", 30}}},
        {"fConfig", path}, {"scColorWidth", 1600}, {"scColorHeight", 1200}, {"pclStride", 2},
        {"scNTPServerIP", "192.168.1.2"}, {"scHDRExposureTimes", {{0,4000},{1,1000}}},
        {"scAIModuleParams", {{123, {0,127,255}}}}, {"scAIModuleInputFrames", {{0,true},{3,false}}}};
    { Camera invalid; invalid.setName("invalid");
      json bad = startup; bad["thread"]["FPS"] = 0;
      assert(!invalid.init(bad)); }
    Camera camera; Replies replies;
    camera.setName("scepter"); assert(camera.init(startup));
    auto command = [&](const char *cmd, json patch = nullptr) {
        camera.console(json{{"cmd",cmd}, {"requestId",17}, {"config",patch}}, &replies);
        assert(replies.last["module"] == "scepter" && replies.last["requestId"] == 17);
        return replies.last;
    };
    const json initial = command("loadConfig");
    assert(initial["bSuccess"] && !initial["deviceOpen"].get<bool>());
    assert(initial["schema"].size() == initial["config"].size() && initial["schema"].size() > 50);
    assert(initial["config"]["scColorAECROIWidth"] == 1600);
    for (const auto &patch : {json{{"scTimeFilterThreshold",7}}, json{{"scFrameRate",0}}, json{{"scScanTime",-1}},
        json{{"scSoftwareTriggerFrameCount",257}}, json{{"scAutoExposureRGB",1}}, json{{"scTimeSyncMode",3}},
        json{{"scHWTriggerWidth",4294967296ULL}}, json{{"scIRGMMGain",2.5}}, json{{"scNTPServerIP","bad"}},
        json{{"scAIModuleParams",{{1,{256}}}}}, json{{"scHDRExposureTimes",{{256,4000}}}},
        json{{"scHDRExposureTimes",{{0,4000},{0,1000}}}}, json{{"scAIModuleInputFrames",{{2,true}}}},
        json{{"scColorGain",nullptr}}, json{{"unknown",1}}, json::array(), json{{"scColorAECROIX",65535}}}) {
        const json reply = command("setConfig", patch);
        assert(!reply["bSuccess"].get<bool>() && reply["config"] == initial["config"]);
    }
    assert(!command("setConfig", {{"scTimeFilterThreshold",2}})["bSuccess"].get<bool>());
    assert(!std::filesystem::exists(path));
    assert(command("saveConfig")["bSuccess"]);
    json saved; std::ifstream(path) >> saved;
    assert(saved == initial["config"]); // full snapshot, including defaults, false and empty collections
    assert(saved["scAIModuleParams"] == startup["scAIModuleParams"]);
    assert(saved["scAIModuleInputFrames"] == startup["scAIModuleInputFrames"]);
    {
        Camera restored; restored.setName("restored"); assert(restored.init(startup));
        json again; assert(restored.saveConfig(again)); assert(again == saved);
        assert(restored.getCamCtrl().m_aiParams.at(123).at(2) == 255);
        std::ofstream(path) << json{{"scTimeFilterThreshold",3}};
        assert(restored.loadConfig(&again)); assert(again["scTimeFilterThreshold"] == 3);
        assert(again["scNTPServerIP"] == "192.168.1.2");
        std::ofstream(path) << json{{"scSoftwareTriggerFrameCount",256}};
        assert(!restored.loadConfig()); assert(restored.getCamCtrl().m_softwareTriggerFrameCount == 1);
        assert(!restored.saveConfig(again, string(argv[1]) + "/missing/controls.json"));
    }
    camera.connected(true); calls.clear();
    assert(command("setConfig", {{"scTimeFilterThreshold",2}})["bSuccess"]);
    assert(calls == std::vector<string>{"scSetTimeFilterParams"});
    assert(camera.getCamCtrl().m_timeFilter.threshold == 2);
    reject = "scSetTimeFilterParams";
    const json failed = command("setConfig", {{"scTimeFilterThreshold",3}});
    assert(!failed["bSuccess"].get<bool>() && failed["config"]["scTimeFilterThreshold"] == 2);
    reject.clear(); calls.clear();
    assert(command("setConfig", {{"scFrameRate",15}})["bSuccess"]);
    assert(calls == std::vector<string>({"scStopStream","scSetFrameRate","scGetSensorIntrinsicParameters","scStartStream"}));
    assert(command("setConfig", {{"scHDR",true}})["config"]["scAutoExposureToF"] == false);
    rejectedHDRindex = 1;
    const auto partial = command("setConfig", {{"scHDRExposureTimes", {{0,5000},{1,2000}}}});
    assert(!partial["bSuccess"].get<bool>());
    assert(camera.getCamCtrl().m_tExposureHDR.at(0) == 5000 && camera.getCamCtrl().m_tExposureHDR.at(1) == 1000);
    rejectedHDRindex = -1;
    assert(command("setConfig", {{"scAutoExposureToF",true}})["config"]["scHDR"] == false);
    // Editing an inactive manual control retains its value without an SDK call.
    calls.clear(); assert(command("setConfig", {{"scExposureTimeRGB",4321}})["bSuccess"]); assert(calls.empty());
    assert(command("setConfig", {{"scAutoExposureRGB",false}})["bSuccess"]);
    assert(std::find(calls.begin(),calls.end(),"scSetExposureTime") != calls.end());
    // Loading from disk applies ALL configured controls, not just differences.
    std::ofstream(path) << json{{"scTimeFilterThreshold",4}};
    calls.clear(); json loaded; assert(camera.loadConfig(&loaded));
    assert(loaded["scTimeFilterThreshold"] == 4);
    for (const char *name : {"scSetTimeFilterParams","scSetFrameRate","scSetToFResolution","scSetColorResolution",
        "scSetConfidenceFilterParams","scSetRealTimeSyncConfig","scAIModuleSetEnabled","scStartStream"})
        assert(std::find(calls.begin(),calls.end(),name) != calls.end());
    json stillOnDisk; std::ifstream(path) >> stillOnDisk;
    assert(stillOnDisk.size() == 1); // live edits and disk loads never save implicitly
    assert(command("saveConfig")["bSuccess"]);
    std::ifstream(path) >> stillOnDisk; assert(stillOnDisk == loaded);
    // Caller-supplied browser paths are ignored.
    camera.console(json{{"cmd","saveConfig"},{"fName",string(argv[1])+"/untrusted.json"}}, &replies);
    assert(!std::filesystem::exists(string(argv[1])+"/untrusted.json"));
    assert(command("setConfig", {{"scHDR",true}})["bSuccess"]);
    std::ofstream(path) << json{{"scAutoExposureToF",true}};
    assert(camera.loadConfig());
    assert(!camera.getCamCtrl().m_bHDR && camera.getCamCtrl().m_bAutoExposureToF);
    camera.connected(false);
    std::cout << "PASS: Scepter full snapshots, reload, validation, offline replies, live setters, failure restoration and stream restart\n";
}
