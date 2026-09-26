#ifndef OpenKAI_test_configLifecycle_visionSaveCoverage_H_
#define OpenKAI_test_configLifecycle_visionSaveCoverage_H_

#if defined(WITH_VISION) && defined(USE_OPENCV)
#include "src/Vision/_Camera.h"
#include "src/Vision/RGBD/_RGBDbase.h"
#include "src/Vision/Pipeline/_Threshold.h"
#include <cassert>
#include <fstream>
#include <iostream>
#endif

namespace kai
{
#if defined(WITH_VISION) && defined(USE_OPENCV)
	class VisionSaveRGBD : public _RGBDbase
	{
	public:
		void changeSettings(void)
		{
			m_devURI = "changed-depth-camera";
			m_devFPS = 24;
			m_bRGB = false;
			m_vSizeRGB = Vector2i(800, 600);
			m_devFPSd = 15;
			m_vSizeD = Vector2i(320, 240);
			m_vRangeD = Vector2f(0.25f, 7.5f);
			m_dScale = 0.125f;
			m_dOfs = -0.5f;
			m_bDepth = false;
			m_bIR = true;
			m_btRGB = true;
			m_btDepth = true;
			m_bConfidence = false;
			m_fConfidenceThr = 0.75f;
			m_bIMU = true;
			m_bPCL = true;
			m_bPCLrgb = true;
		}
	};

	class VisionSaveCamera : public _Camera
	{
	public:
		void changeSettings(void)
		{
			m_devURI = "changed-color-camera";
			m_devFPS = 19;
			m_bRGB = false;
			m_vSizeRGB = Vector2i(960, 540);
			m_deviceID = 7;
			m_nInitRead = 5;
			m_bResetCam = true;
		}
	};

	class VisionSaveThreshold : public _Threshold
	{
	public:
		void changeSettings(void)
		{
			assert(m_vFilter.size() == 1);
			assert(m_vFilter[0].m_name == "edge");
			m_vFilter[0].m_type = img_thr;
			m_vFilter[0].m_vMax = 255.0;
			m_vFilter[0].m_bAutoThr = true;
			m_vFilter[0].m_thr = 123.0;
			m_vFilter[0].m_method = ADAPTIVE_THRESH_MEAN_C;
			m_vFilter[0].m_thrType = THRESH_BINARY_INV;
			m_vFilter[0].m_blockSize = 9;
			m_vFilter[0].m_C = 2.5;
		}
	};

	static void visionSaveBind(BASE &module, ModuleMgr &manager, JsonCfg &cfg, const string &name)
	{
		module.setModuleMgr(&manager);
		module.setName(name);
		module.setConfig(&cfg, &(*cfg.getJson())[name]);
		assert(module.loadConfig());
	}

	static void visionSaveErase(json &j, const json &values)
	{
		for (const auto &field : values.items())
		{
			j.erase(field.key());
		}
	}

	static void visionSaveAssertValues(const json &j, const json &values)
	{
		for (const auto &field : values.items())
		{
			assert(j.at(field.key()) == field.value());
		}
	}

	template <typename T>
	static void visionSaveRoundTrip(T &module, ModuleMgr &manager, JsonCfg &cfg,
		const string &name, const json &values, const string &section = "")
	{
		json &configured = (*cfg.getJson())[name];
		json &fields = section.empty() ? configured : configured["filters"][section];
		visionSaveErase(fields, values);
		assert(module.saveConfig(true));

		JsonCfg reloadedCfg;
		assert(reloadedCfg.readFromFile("vision-save.json"));
		json &saved = (*reloadedCfg.getJson())[name];
		json &savedFields = section.empty() ? saved : saved["filters"][section];
		visionSaveAssertValues(savedFields, values);
		assert(saved["custom"] == "keep module metadata");
		assert(saved["thread"]["custom"] == "keep thread metadata");
		assert((*reloadedCfg.getJson())["sibling"] == "keep sibling");
		if (!section.empty())
		{
			assert(savedFields["units"] == "keep filter metadata");
			assert(saved["filters"]["/comment"] == "keep ignored entry");
			assert(saved["_VisionBase"] == "camera");
		}

		// A fresh object must reload the values, then reproduce them without JSON fallback.
		T reloaded;
		visionSaveBind(reloaded, manager, reloadedCfg, name);
		visionSaveErase(savedFields, values);
		assert(reloaded.saveConfig(true));
		JsonCfg verified;
		assert(verified.readFromFile("vision-save.json"));
		const json &verifiedModule = (*verified.getJson())[name];
		visionSaveAssertValues(section.empty() ? verifiedModule : verifiedModule["filters"][section], values);
	}
#endif

	static void testVisionSaveCoverage(void)
	{
#if defined(WITH_VISION) && defined(USE_OPENCV)
		std::cout << "Vision inherited settings and named filter save round trips" << std::endl;
		json fixture = {{"sibling", "keep sibling"}};
		for (const char *name : {"rgbd", "camera", "threshold"})
		{
			fixture[name] = {{"class", name}, {"custom", "keep module metadata"},
				{"thread", {{"FPS", 11}, {"custom", "keep thread metadata"}}}};
		}
		fixture["rgbd"]["class"] = "_RGBDbase";
		fixture["rgbd"].update({{"devURI", "initial-depth-camera"}, {"devFPS", 30},
			{"bRGB", true}, {"vSizeRGB", {640, 480}}, {"devFPSd", 30}, {"vSizeD", {640, 480}},
			{"vRangeD", {0.0, 4.0}}, {"dScale", 1.0}, {"dOfs", 0.0}, {"bDepth", true},
			{"bIR", false}, {"btRGB", false}, {"btDepth", false}, {"bConfidence", true},
			{"fConfidenceThr", 0.0}, {"bIMU", false}, {"bPCL", false}, {"bPCLrgb", false}});
		fixture["camera"]["class"] = "_Camera";
		fixture["camera"].update({{"devURI", "initial-color-camera"}, {"devFPS", 30},
			{"bRGB", true}, {"vSizeRGB", {640, 480}}, {"deviceID", 0},
			{"nInitRead", 1}, {"bResetCam", false}});
		fixture["threshold"]["class"] = "_Threshold";
		fixture["threshold"]["_VisionBase"] = "camera";
		fixture["threshold"]["filters"] = {{"edge", {{"thr", 4}, {"units", "keep filter metadata"}}},
			{"/comment", "keep ignored entry"}};
		std::ofstream output("vision-save.json");
		output << fixture.dump(2);
		output.close();
		assert(output.good());

		ModuleMgr manager;
		JsonCfg cfg;
		assert(cfg.readFromFile("vision-save.json"));
		VisionSaveRGBD rgbd;
		visionSaveBind(rgbd, manager, cfg, "rgbd");
		rgbd.changeSettings();
		visionSaveRoundTrip(rgbd, manager, cfg, "rgbd", {
			{"devURI", "changed-depth-camera"}, {"devFPS", 24}, {"bRGB", false}, {"vSizeRGB", {800, 600}},
			{"devFPSd", 15}, {"vSizeD", {320, 240}}, {"vRangeD", {0.25, 7.5}},
			{"dScale", 0.125}, {"dOfs", -0.5}, {"bDepth", false}, {"bIR", true},
			{"btRGB", true}, {"btDepth", true}, {"bConfidence", false}, {"fConfidenceThr", 0.75},
			{"bIMU", true}, {"bPCL", true}, {"bPCLrgb", true}});

		VisionSaveCamera camera;
		visionSaveBind(camera, manager, cfg, "camera");
		camera.changeSettings();
		visionSaveRoundTrip(camera, manager, cfg, "camera", {
			{"devURI", "changed-color-camera"}, {"devFPS", 19}, {"bRGB", false}, {"vSizeRGB", {960, 540}},
			{"deviceID", 7}, {"nInitRead", 5}, {"bResetCam", true}});

		VisionSaveThreshold threshold;
		visionSaveBind(threshold, manager, cfg, "threshold");
		threshold.changeSettings();
		visionSaveRoundTrip(threshold, manager, cfg, "threshold", {
			{"type", img_thr}, {"vMax", 255.0}, {"bAutoThr", true}, {"thr", 123.0},
			{"method", ADAPTIVE_THRESH_MEAN_C}, {"thrType", THRESH_BINARY_INV},
			{"blockSize", 9}, {"C", 2.5}}, "edge");
#endif
	}
}

#endif
