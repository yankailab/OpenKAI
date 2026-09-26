/*
 * _Scepter.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_Scepter.h"
#include <cmath>
#include <set>
#include <type_traits>
#include <arpa/inet.h>

namespace kai
{
	namespace
	{

		// One field list drives JSON persistence, validation and the browser schema.
		// Function pointers (hot-plug callback/user data) stay in C++ only.
		template <typename F>
		void visitScControls(ScCtrl &c, F f)
		{
			const char *ip = reinterpret_cast<const char *>(c.m_timeSync.ip);
			string ntpIP(ip, strnlen(ip, sizeof(c.m_timeSync.ip)));
			f("scScanTime", c.m_tScan, "Streams and point cloud");
			f("scPixelFormat", c.m_pixelFormat, "Streams and point cloud");
			f("scFrameRate", c.m_frameRate, "Streams and point cloud");
			f("scToFWidth", c.m_resolutionToF.width, "Streams and point cloud");
			f("scToFHeight", c.m_resolutionToF.height, "Streams and point cloud");
			f("scColorWidth", c.m_resolutionRGB.width, "Streams and point cloud");
			f("scColorHeight", c.m_resolutionRGB.height, "Streams and point cloud");
			f("scTransformColorToDepth", c.m_bTransformRGBToDepth, "Streams and point cloud");
			f("scTransformDepthToColor", c.m_bTransformDepthToRGB, "Streams and point cloud");
			f("scWorkMode", c.m_workMode, "Trigger");
			f("scSoftwareTriggerFrameCount", c.m_softwareTriggerFrameCount, "Trigger");
			f("scHWTriggerWidth", c.m_hwTrigger.width, "Trigger");
			f("scHWTriggerInterval", c.m_hwTrigger.interval, "Trigger");
			f("scHWTriggerPolarity", c.m_hwTrigger.polarity, "Trigger");
			f("scAutoExposureToF", c.m_bAutoExposureToF, "ToF exposure");
			f("scExposureTimeToF", c.m_tExposureToF, "ToF exposure");
			f("scHDR", c.m_bHDR, "ToF exposure");
			f("scHDRExposureTimes", c.m_tExposureHDR, "ToF exposure");
			f("scWDR", c.m_bWDR, "ToF exposure");
			f("scWDRExposureTimes", c.m_tExposureWDR, "ToF exposure");
			f("scAutoExposureRGB", c.m_bAutoExposureRGB, "Color");
			f("scExposureTimeRGB", c.m_tExposureRGB, "Color");
			f("scColorGain", c.m_gainRGB, "Color");
			f("scColorAECMaxExposureTime", c.m_tAECMaxExposureRGB, "Color");
			f("scColorAECROIX", c.m_aecROIRGBorigin.x, "Color");
			f("scColorAECROIY", c.m_aecROIRGBorigin.y, "Color");
			f("scColorAECROIWidth", c.m_aecROIRGBsize.width, "Color");
			f("scColorAECROIHeight", c.m_aecROIRGBsize.height, "Color");
			f("scTimeFilterEnabled", c.m_timeFilter.enable, "Filters and IR");
			f("scTimeFilterThreshold", c.m_timeFilter.threshold, "Filters and IR");
			f("scConfidenceFilterEnabled", c.m_confidenceFilter.enable, "Filters and IR");
			f("scConfidenceFilterThreshold", c.m_confidenceFilter.threshold, "Filters and IR");
			f("scFlyingPixelFilterEnabled", c.m_flyingPixelFilter.enable, "Filters and IR");
			f("scFlyingPixelFilterThreshold", c.m_flyingPixelFilter.threshold, "Filters and IR");
			f("scFillHoleEnabled", c.m_bFillHole, "Filters and IR");
			f("scSpatialFilterEnabled", c.m_bSpatialFilter, "Filters and IR");
			f("scIRGMMGain", c.m_irGMMGain, "Filters and IR");
			f("scIRGMMCorrectionEnabled", c.m_irGMMCorrection.enable, "Filters and IR");
			f("scIRGMMCorrectionThreshold", c.m_irGMMCorrection.threshold, "Filters and IR");
			f("scDHCPEnabled", c.m_bDHCP, "Network and time");
			f("scDeviceIPAddr", c.m_deviceIPAddr, "Network and time");
			f("scDeviceSubnetMask", c.m_deviceSubnetMask, "Network and time");
			f("scTimeSyncMode", c.m_timeSync.flag, "Network and time");
			f("scNTPServerIP", ntpIP, "Network and time");
			f("scAIModuleEnabled", c.m_bAIModule, "AI module");
			f("scAIModuleWorkMode", c.m_aiWorkMode, "AI module");
			f("scAIModuleParams", c.m_aiParams, "AI module");
			f("scAIModuleInputFrames", c.m_aiInputFrames, "AI module");
			f("scAIModulePreviewFrames", c.m_aiPreviewFrames, "AI module");
			f("scParamsJsonFile", c.m_paramsJsonFile, "SDK preset");
			memset(c.m_timeSync.ip, 0, sizeof(c.m_timeSync.ip));
			memcpy(c.m_timeSync.ip, ntpIP.data(), std::min(ntpIP.size(), sizeof(c.m_timeSync.ip) - 1));
		}

		template <typename T>
		json scFieldSpec(const char *key, const T &, const char *category)
		{
			json spec = {{"key", key}, {"category", category}};
			if constexpr (std::is_same_v<T, bool>) spec["type"] = "bool";
			else if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
			{
				spec["type"] = "int";
				spec["min"] = 0;
				if constexpr (std::is_enum_v<T>) spec["max"] = 2;
				else spec["max"] = std::numeric_limits<T>::max();
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				spec["type"] = "float"; spec["min"] = 0.000001; spec["max"] = std::numeric_limits<T>::max();
			}
			else if constexpr (std::is_same_v<T, string>) spec["type"] = "string";
			else spec["type"] = "pairs";
			const string k(key);
			if (k == "scTimeFilterThreshold") { spec["min"] = 1; spec["max"] = 6; }
			if (k == "scConfidenceFilterThreshold" || k == "scIRGMMCorrectionThreshold") { spec["min"] = 1; spec["max"] = 100; }
			if (k == "scFlyingPixelFilterThreshold") { spec["min"] = 1; spec["max"] = 16; }
			if (k == "scSoftwareTriggerFrameCount") { spec["min"] = 1; spec["max"] = 10; }
			if (k == "scHWTriggerPolarity") spec["max"] = 1;
			if (k == "scTimeSyncMode") spec["max"] = 2;
			if (k == "scPixelFormat") { spec["min"] = 3; spec["max"] = 6; spec["hint"] = "3: RGB JPEG, 4: BGR JPEG, 5: RGB, 6: BGR"; }
			if (k == "scWorkMode") spec["hint"] = "0: active, 1: hardware trigger, 2: software trigger";
			if (k == "scAIModuleWorkMode") spec["hint"] = "0: continuous, 1: single run, 2: single report";
			if (k == "scTimeSyncMode") spec["hint"] = "0: off, 1: NTP, 2: PTP";
			if (k == "scFrameRate" || k == "scScanTime" || k.find("ExposureTime") != string::npos ||
				k == "scToFWidth" || k == "scToFHeight" || k == "scColorWidth" || k == "scColorHeight" ||
				k == "scColorAECROIWidth" || k == "scColorAECROIHeight") spec["min"] = 1;
			if (k == "scDeviceIPAddr" || k == "scDeviceSubnetMask" || k == "scNTPServerIP") spec["maxLength"] = 15;
			if (k == "scParamsJsonFile") spec["hint"] = "Optional SDK preset path on the backend; applied before explicit controls.";
			if (k == "scHDRExposureTimes" || k == "scWDRExposureTimes")
			{
				spec["example"] = json::array({json::array({0, 4000}), json::array({1, 1000})});
				spec["hint"] = "JSON pairs [frame index, exposure in microseconds].";
			}
			if (k == "scAIModuleParams") { spec["example"] = json::array({json::array({0, json::array({1, 2})})}); spec["hint"] = "JSON pairs [parameter ID, byte array]; IDs depend on the camera."; }
			if (k == "scAIModuleInputFrames" || k == "scAIModulePreviewFrames")
			{
				spec["example"] = json::array({json::array({0, true})});
				spec["hint"] = "JSON pairs [frame type, enabled]. Types: 0 depth, 1 IR, 3 color, 4 aligned color, 5 aligned depth.";
			}
			if (k == "scFrameRate" || k == "scWorkMode" || k == "scPixelFormat" ||
				k == "scToFWidth" || k == "scToFHeight" || k == "scColorWidth" || k == "scColorHeight" ||
				k == "scTransformColorToDepth" || k == "scTransformDepthToColor" || k == "scParamsJsonFile") spec["restart"] = true;
			return spec;
		}

		bool scInteger(const json &v, double low, double high)
		{
			return v.is_number_integer() && v.get<double>() >= low && v.get<double>() <= high;
		}

		void validateScValue(const json &v, const json &spec)
		{
			const string type = spec["type"], key = spec["key"];
			bool valid = false;
			if (type == "bool") valid = v.is_boolean();
			else if (type == "int") valid = scInteger(v, spec["min"].get<double>(), spec["max"].get<double>());
			else if (type == "float") valid = v.is_number() && std::isfinite(v.get<double>()) && v >= spec["min"] && v <= spec["max"];
			else if (type == "string")
			{
				valid = v.is_string();
				if (valid)
				{
					const auto &text = v.get_ref<const string &>();
					valid = text.find('\0') == string::npos && text.size() <= spec.value("maxLength", size_t(4096));
					if (valid && spec.contains("maxLength") && !text.empty())
					{
						struct in_addr address;
						valid = inet_pton(AF_INET, text.c_str(), &address) == 1;
					}
				}
			}
			else if (type == "pairs")
			{
				valid = v.is_array();
				std::set<uint32_t> seen;
				if (valid) for (const auto &pair : v)
				{
					const bool bytes = key == "scAIModuleParams";
					const bool frames = key == "scAIModuleInputFrames" || key == "scAIModulePreviewFrames";
					if (!pair.is_array() || pair.size() != 2 || !scInteger(pair[0], 0, bytes ? UINT32_MAX : frames ? 5 : UINT8_MAX)) { valid = false; break; }
					const uint32_t index = pair[0].get<uint32_t>();
					if (!seen.insert(index).second || (frames && index == 2)) { valid = false; break; }
					if (bytes)
					{
						if (!pair[1].is_array() || pair[1].size() > UINT16_MAX) { valid = false; break; }
						for (const auto &byte : pair[1]) if (!scInteger(byte, 0, UINT8_MAX)) valid = false;
					}
					else if (frames ? !pair[1].is_boolean() : !scInteger(pair[1], 1, INT32_MAX)) valid = false;
				}
			}
			if (!valid) throw std::invalid_argument("Invalid value for " + key);
		}

		bool copyScFrame(const ScFrame &frame, Mat &image)
		{
			if (!frame.pFrameData || !frame.width || !frame.height)
				return false;

			int type;
			switch (frame.pixelFormat)
			{
			case SC_PIXEL_FORMAT_DEPTH_MM16:
				type = CV_16UC1;
				break;
			case SC_PIXEL_FORMAT_GRAY_8:
				type = CV_8UC1;
				break;
			case SC_PIXEL_FORMAT_BGR_888:
			case SC_PIXEL_FORMAT_BGR_888_JPEG:
			case SC_PIXEL_FORMAT_RGB_888:
			case SC_PIXEL_FORMAT_RGB_888_JPEG:
				type = CV_8UC3;
				break;
			default:
				return false;
			}
			if (frame.dataLen < size_t(frame.width) * frame.height * CV_ELEM_SIZE(type))
				return false;

			Mat view(frame.height, frame.width, type, frame.pFrameData);
			if (frame.pixelFormat == SC_PIXEL_FORMAT_RGB_888 || frame.pixelFormat == SC_PIXEL_FORMAT_RGB_888_JPEG)
				cv::cvtColor(view, image, cv::COLOR_RGB2BGR);
			else
				view.copyTo(image);

			// Scepter marks invalid depth with both 0 and 65535. Exposing the
			// latter as a distance destroys min/max depth-preview contrast.
			if (frame.pixelFormat == SC_PIXEL_FORMAT_DEPTH_MM16)
				image.setTo(0, image == UINT16_MAX);
			return true;
		}
	}

	_Scepter::_Scepter()
	{
		m_vRangeD = {0.0f, 5.0f};
		m_dScale = 0.001f; // SDK depth is in millimeters; measurements use meters.
	}

	_Scepter::~_Scepter()
	{
		if (m_pT)
			m_pT->join();
		if (m_pTpp)
			m_pTpp->join();
		close();
		if (m_bScInitialized)
			scShutdown();
	}

	bool _Scepter::loadConfig(void)
	{
		if (!_RGBDbase::loadConfig())
		{
			return false;
		}
		const json &j = *m_pJ;

		json startup = json::object();
		for (const auto &spec : controlSchema())
		{
			const string key = spec["key"];
			auto it = j.find(key);
			if (it != j.end())
			{
				startup[key] = *it;
			}
		}

		// Explicit sc options take precedence over the common RGBD settings.
		if (const json *pJthread = jK(j, "thread"))
		{
			jKv(*pJthread, "FPS", m_scCtrl.m_frameRate);
		}
		m_scCtrl.m_resolutionToF = {(uint16_t)m_vSizeD.x(), (uint16_t)m_vSizeD.y()};
		if (!j.contains("vSizeD"))
		{
			m_scCtrl.m_resolutionToF = {640, 480};
		}
		m_scCtrl.m_resolutionRGB = {(uint16_t)m_vSizeRGB.x(), (uint16_t)m_vSizeRGB.y()};
		m_scCtrl.m_aecROIRGBsize = m_scCtrl.m_resolutionRGB;
		m_scCtrl.m_bTransformRGBToDepth = m_btRGB;
		m_scCtrl.m_bTransformDepthToRGB = m_btDepth;
		if (!startup.contains("scFrameRate"))
		{
			startup["scFrameRate"] = m_scCtrl.m_frameRate;
		}
		json errors;
		if (!applyConfig(startup, false, errors))
		{
			LOG_E(errors.dump());
			return false;
		}

		DEL(m_pTpp);
		m_pTpp = createThread(jK(*m_pJ, "threadPP"), "threadPP");
		NULL_F(m_pTpp);

		return true;
	}

	bool _Scepter::link(void)
	{
		if (!_RGBDbase::link() || !m_pTpp || !m_pTpp->link())
		{
			return false;
		}

		return true;
	}

	json _Scepter::configValues(void)
	{
		json j = {{"bRGB", m_bRGB}, {"bDepth", m_bDepth}, {"bIR", m_bIR},
			{"bPCL", m_bPCL}, {"bPCLrgb", m_bPCLrgb}, {"pclStride", m_pclStride}, {"dScale", m_dScale}};
		visitScControls(m_scCtrl, [&](const char *key, auto &value, const char *) { j[key] = value; });
		return j;
	}

	json _Scepter::controlSchema(void)
	{
		json schema = json::array();
		for (const char *key : {"bRGB", "bDepth", "bIR", "bPCL", "bPCLrgb"})
			schema.push_back(scFieldSpec(key, false, "Streams and point cloud"));
		json stride = scFieldSpec("pclStride", uint16_t(1), "Streams and point cloud");
		stride["min"] = 1; schema.push_back(stride);
		schema.push_back(scFieldSpec("dScale", 1.0f, "Streams and point cloud"));
		visitScControls(m_scCtrl, [&](const char *key, auto &value, const char *category) {
			schema.push_back(scFieldSpec(key, value, category));
		});
		return schema;
	}

	bool _Scepter::applyConfig(const json &patch, bool device, json &errors)
	{
		errors = json::object();
		if (!patch.is_object()) { errors["config"] = "Expected a JSON object"; return false; }
		json specs = json::object();
		for (const auto &spec : controlSchema()) specs[spec["key"].get<string>()] = spec;
		for (auto it = patch.begin(); it != patch.end(); ++it)
		{
			try
			{
				if (!specs.contains(it.key())) throw std::invalid_argument("Unknown parameter");
				validateScValue(it.value(), specs[it.key()]);
			}
			catch (const std::exception &e) { errors[it.key()] = e.what(); }
		}
		if (!errors.empty()) return false;
		if (device && !m_bOpened) { errors["device"] = "Camera is not open"; return false; }

		ScCtrl candidate = m_scCtrl;
		visitScControls(candidate, [&](const char *key, auto &value, const char *) {
			if (patch.contains(key)) value = patch[key].get<std::decay_t<decltype(value)>>();
		});
		// HDR and auto ToF exposure are mutually exclusive. Selecting either mode
		// updates the paired control in the reply and in the complete saved snapshot.
		json changed = patch;
		if (patch.value("scHDR", false)) { candidate.m_bAutoExposureToF = false; changed["scAutoExposureToF"] = false; }
		else if (patch.value("scAutoExposureToF", false)) { candidate.m_bHDR = false; changed["scHDR"] = false; }
		// Keep a full-frame AEC ROI full-frame when changing color resolution.
		if (m_scCtrl.m_aecROIRGBorigin.x == 0 && m_scCtrl.m_aecROIRGBorigin.y == 0 &&
			m_scCtrl.m_aecROIRGBsize.width == m_scCtrl.m_resolutionRGB.width &&
			m_scCtrl.m_aecROIRGBsize.height == m_scCtrl.m_resolutionRGB.height)
		{
			if (patch.contains("scColorWidth") && !patch.contains("scColorAECROIWidth"))
			{
				candidate.m_aecROIRGBsize.width = candidate.m_resolutionRGB.width;
				changed["scColorAECROIWidth"] = candidate.m_aecROIRGBsize.width;
			}
			if (patch.contains("scColorHeight") && !patch.contains("scColorAECROIHeight"))
			{
				candidate.m_aecROIRGBsize.height = candidate.m_resolutionRGB.height;
				changed["scColorAECROIHeight"] = candidate.m_aecROIRGBsize.height;
			}
		}
		if (uint32_t(candidate.m_aecROIRGBorigin.x) + candidate.m_aecROIRGBsize.width > candidate.m_resolutionRGB.width ||
			uint32_t(candidate.m_aecROIRGBorigin.y) + candidate.m_aecROIRGBsize.height > candidate.m_resolutionRGB.height)
		{
			errors["scColorAECROIWidth"] = "Color AEC ROI must fit inside the color resolution";
			return false;
		}
		bool restart = false;
		for (auto it = changed.begin(); it != changed.end(); ++it)
			if (specs[it.key()].value("restart", false)) restart = true;
		if (device && restart && scStopStream(m_scDevHandle) != SC_OK)
		{
			errors["device"] = "Could not stop capture to change stream settings";
			return false;
		}
		if (device)
		{
			m_bPCLframe = false;
			applyScControls(candidate, changed, false, errors);
			if (restart)
			{
				if (scGetSensorIntrinsicParameters(m_scDevHandle, SC_TOF_SENSOR, &m_scCamParams) != SC_OK)
					errors["calibration"] = "Could not refresh camera calibration";
				if (scStartStream(m_scDevHandle) != SC_OK)
				{
					errors["device"] = "Could not restart camera capture";
					close();
				}
			}
		}
		else
		{
			m_scCtrl = candidate;
			m_btRGB = candidate.m_bTransformRGBToDepth;
			m_btDepth = candidate.m_bTransformDepthToRGB;
			m_vSizeD = Vector2i(candidate.m_resolutionToF.width, candidate.m_resolutionToF.height);
			m_vSizeRGB = Vector2i(candidate.m_resolutionRGB.width, candidate.m_resolutionRGB.height);
		}
		jKv(patch, "bRGB", m_bRGB); jKv(patch, "bDepth", m_bDepth); jKv(patch, "bIR", m_bIR);
		jKv(patch, "bPCL", m_bPCL); jKv(patch, "bPCLrgb", m_bPCLrgb);
		jKv(patch, "pclStride", m_pclStride); jKv(patch, "dScale", m_dScale);
		return errors.empty();
	}

	bool _Scepter::saveConfig(bool bExport)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		if (!_RGBDbase::saveConfig(false))
		{
			return false;
		}

		m_pJ->update(configValues());

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	ScCtrl _Scepter::getCamCtrl(void)
	{
		return m_scCtrl;
	}

	bool _Scepter::setCamCtrl(const ScCtrl &camCtrl)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		ScCtrl candidate = camCtrl;
		json patch = json::object(), errors;
		visitScControls(candidate, [&](const char *key, auto &value, const char *) { patch[key] = value; });
		if (!applyConfig(patch, m_bOpened, errors)) return false;
		if (camCtrl.m_hotPlugCallback != m_scCtrl.m_hotPlugCallback || camCtrl.m_pHotPlugUserData != m_scCtrl.m_pHotPlugUserData)
			return setHotPlugStatusCallback(camCtrl.m_hotPlugCallback, camCtrl.m_pHotPlugUserData);
		return true;
	}

	void _Scepter::applyScControls(const ScCtrl &requested, const json &changed, bool all, json &errors)
	{
		// Setters cache their requested value before calling the SDK. On failure,
		// restore that control's previous value so the browser sees accepted state.
		const json before = configValues();
		const ScCtrl c = requested; // callers may pass m_scCtrl itself
		m_scCtrl = c;
		if (!c.m_paramsJsonFile.empty() && changed.contains("scParamsJsonFile")) all = true;
		json selectedKeys = changed;
		auto dependent = [&](const char *trigger, std::initializer_list<const char *> keys) {
			if (changed.contains(trigger)) for (const char *key : keys) selectedKeys[key] = true;
		};
		dependent("scWorkMode", {"scSoftwareTriggerFrameCount", "scHWTriggerWidth"});
		dependent("scAutoExposureToF", {"scExposureTimeToF"});
		dependent("scHDR", {"scHDRExposureTimes"});
		dependent("scWDR", {"scWDRExposureTimes"});
		dependent("scAutoExposureRGB", {"scExposureTimeRGB", "scColorGain", "scColorAECMaxExposureTime", "scColorAECROIX"});
		dependent("scAIModuleEnabled", {"scAIModuleWorkMode", "scAIModuleParams", "scAIModuleInputFrames", "scAIModulePreviewFrames"});
		dependent("scDHCPEnabled", {"scDeviceIPAddr", "scDeviceSubnetMask"});
		auto apply = [&](std::initializer_list<const char *> keys, bool active, auto setter) {
			bool selected = all;
			for (const char *key : keys) selected |= selectedKeys.contains(key);
			if (!selected || !active || setter()) return;
			for (const char *key : keys) errors[key] = "Device rejected this value or does not support this control";
			visitScControls(m_scCtrl, [&](const char *key, auto &value, const char *) {
				for (const char *failed : keys)
					if (string(key) == failed) value = before[key].get<std::decay_t<decltype(value)>>();
			});
		};
		// Indexed controls can fail independently. Keep successful entries and
		// restore only the rejected index, rather than undoing the whole cache.
		auto applyEntries = [&](const char *key, bool active, auto &stored, const auto &entries, auto setter) {
			if ((!all && !selectedKeys.contains(key)) || !active) return;
			const auto previous = before[key].get<std::decay_t<decltype(stored)>>();
			for (const auto &entry : entries)
			{
				if (setter(entry.first, entry.second)) continue;
				errors[key] = "Device rejected one or more indexed values";
				const auto old = previous.find(entry.first);
				if (old == previous.end()) stored.erase(entry.first);
				else stored[entry.first] = old->second;
			}
		};
		apply({"scParamsJsonFile"}, !c.m_paramsJsonFile.empty(), [&] { return setParamsByJson(c.m_paramsJsonFile); });
		apply({"scFrameRate"}, true, [&] { return setFrameRate(c.m_frameRate); });
		apply({"scWorkMode"}, true, [&] { return setWorkMode(c.m_workMode); });
		apply({"scSoftwareTriggerFrameCount"}, m_scCtrl.m_workMode == SC_SOFTWARE_TRIGGER_MODE,
			[&] { return setSoftwareTriggerParameter(c.m_softwareTriggerFrameCount); });
		apply({"scHWTriggerWidth", "scHWTriggerInterval", "scHWTriggerPolarity"}, m_scCtrl.m_workMode == SC_HARDWARE_TRIGGER_MODE,
			[&] { return setInputSignalParamsForHWTrigger(c.m_hwTrigger.width, c.m_hwTrigger.interval, c.m_hwTrigger.polarity); });
		apply({"scToFWidth", "scToFHeight"}, true, [&] { return setToFResolution(c.m_resolutionToF.width, c.m_resolutionToF.height); });
		apply({"scColorWidth", "scColorHeight"}, true, [&] { return setColorResolution(c.m_resolutionRGB.width, c.m_resolutionRGB.height); });
		apply({"scPixelFormat"}, true, [&] { return setColorPixelFormat((ScPixelFormat)c.m_pixelFormat); });
		apply({"scTransformColorToDepth"}, true, [&] { return setTransformColorImgToDepthSensorEnabled(c.m_bTransformRGBToDepth); });
		apply({"scTransformDepthToColor"}, true, [&] { return setTransformDepthImgToColorSensorEnabled(c.m_bTransformDepthToRGB); });
		apply({"scHDR"}, !c.m_bHDR, [&] { return setHDR(false); });
		apply({"scAutoExposureToF"}, true, [&] { return setToFexposureControlMode(c.m_bAutoExposureToF); });
		apply({"scExposureTimeToF"}, !m_scCtrl.m_bAutoExposureToF,
			[&] { return setToFexposureTime(c.m_tExposureToF); });
		apply({"scHDR"}, c.m_bHDR, [&] { return setHDR(true); });
		applyEntries("scHDRExposureTimes", m_scCtrl.m_bHDR, m_scCtrl.m_tExposureHDR, c.m_tExposureHDR,
			[&](auto index, const auto &value) { return setExposureTimeOfHDR(index, value); });
		apply({"scWDR"}, true, [&] { return setWDR(c.m_bWDR); });
		applyEntries("scWDRExposureTimes", m_scCtrl.m_bWDR, m_scCtrl.m_tExposureWDR, c.m_tExposureWDR,
			[&](auto index, const auto &value) { return setExposureTimeOfWDR(index, value); });
		apply({"scAutoExposureRGB"}, true, [&] { return setRGBexposureControlMode(c.m_bAutoExposureRGB); });
		apply({"scExposureTimeRGB"}, !m_scCtrl.m_bAutoExposureRGB, [&] { return setRGBexposureTime(c.m_tExposureRGB); });
		apply({"scColorGain"}, !m_scCtrl.m_bAutoExposureRGB, [&] { return setColorGain(c.m_gainRGB); });
		apply({"scColorAECMaxExposureTime"}, m_scCtrl.m_bAutoExposureRGB,
			[&] { return setColorAECMaxExposureTime(c.m_tAECMaxExposureRGB); });
		apply({"scColorAECROIX", "scColorAECROIY", "scColorAECROIWidth", "scColorAECROIHeight"}, m_scCtrl.m_bAutoExposureRGB,
			[&] { return setColorAECROI(c.m_aecROIRGBorigin.x, c.m_aecROIRGBorigin.y, c.m_aecROIRGBsize.width, c.m_aecROIRGBsize.height); });
		apply({"scIRGMMGain"}, true, [&] { return setIRGMMGain(c.m_irGMMGain); });
		apply({"scIRGMMCorrectionEnabled", "scIRGMMCorrectionThreshold"}, true, [&] { return setIRGMMCorrection(c.m_irGMMCorrection.enable, c.m_irGMMCorrection.threshold); });
		apply({"scTimeFilterEnabled", "scTimeFilterThreshold"}, true, [&] { return setTimeFilter(c.m_timeFilter.enable, c.m_timeFilter.threshold); });
		apply({"scConfidenceFilterEnabled", "scConfidenceFilterThreshold"}, true, [&] { return setConfidenceFilter(c.m_confidenceFilter.enable, c.m_confidenceFilter.threshold); });
		apply({"scFlyingPixelFilterEnabled", "scFlyingPixelFilterThreshold"}, true, [&] { return setFlyingPixelFilter(c.m_flyingPixelFilter.enable, c.m_flyingPixelFilter.threshold); });
		apply({"scFillHoleEnabled"}, true, [&] { return setFillHole(c.m_bFillHole); });
		apply({"scSpatialFilterEnabled"}, true, [&] { return setSpatialFilter(c.m_bSpatialFilter); });
		apply({"scTimeSyncMode", "scNTPServerIP"}, true, [&] { return setRealTimeSyncConfig(c.m_timeSync); });
		apply({"scAIModuleWorkMode"}, c.m_bAIModule, [&] { return setAIModuleWorkMode(c.m_aiWorkMode); });
		applyEntries("scAIModuleParams", c.m_bAIModule, m_scCtrl.m_aiParams, c.m_aiParams,
			[&](auto index, const auto &value) { return setAIModuleParam(index, value); });
		applyEntries("scAIModuleInputFrames", c.m_bAIModule, m_scCtrl.m_aiInputFrames, c.m_aiInputFrames,
			[&](auto index, const auto &value) { return setAIModuleInputFrameTypeEnabled(index, value); });
		applyEntries("scAIModulePreviewFrames", c.m_bAIModule, m_scCtrl.m_aiPreviewFrames, c.m_aiPreviewFrames,
			[&](auto index, const auto &value) { return setAIModulePreviewFrameTypeEnabled(index, value); });
		apply({"scAIModuleEnabled"}, true, [&] { return setAIModuleEnabled(c.m_bAIModule); });
		// Network changes go last because they can disconnect the camera.
		apply({"scDHCPEnabled"}, true, [&] { return setDeviceDHCPEnabled(c.m_bDHCP); });
		apply({"scDeviceSubnetMask"}, !c.m_bDHCP && !c.m_deviceSubnetMask.empty(), [&] { return setDeviceSubnetMask(c.m_deviceSubnetMask); });
		apply({"scDeviceIPAddr"}, !c.m_bDHCP && !c.m_deviceIPAddr.empty(), [&] { return setDeviceIPAddr(c.m_deviceIPAddr); });
	}

	bool _Scepter::open(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		IF__(m_bOpened, true);

		if (m_scCtrl.m_hotPlugCallback)
			setHotPlugStatusCallback(m_scCtrl.m_hotPlugCallback, m_scCtrl.m_pHotPlugUserData);

		m_nDevice = 0;
		ScStatus status = scGetDeviceCount(&m_nDevice, m_scCtrl.m_tScan);
		if (status != ScStatus::SC_OK)
		{
			LOG_E("ScGetDeviceCount failed");
			return false;
		}

		LOG_I("Get device count: " + i2str(m_nDevice));
		IF_F(m_nDevice == 0);

		delete[] m_pScDevListInfo;
		m_pScDevListInfo = new ScDeviceInfo[m_nDevice];
		status = scGetDeviceInfoList(m_nDevice, m_pScDevListInfo);
		if (status != ScStatus::SC_OK)
		{
			LOG_E("GetDeviceListInfo failed:" + i2str(status));
			return false;
		}

		m_scDevHandle = 0;
		if (m_devURI.empty())
			m_devURI = string(m_pScDevListInfo[0].ip);

		LOG_I("Device URI: " + m_devURI);

		status = scOpenDeviceByIP(m_devURI.c_str(), &m_scDevHandle);
		if (status != ScStatus::SC_OK)
		{
			LOG_E("OpenDevice failed");
			return false;
		}

		json errors = json::object();
		applyScControls(m_scCtrl, json::object(), true, errors);
		// Some cameras lack optional controls; keep capture usable and report
		// unsupported settings when those controls are edited through the console.
		if (!errors.empty()) LOG_I("Scepter controls: " + errors.dump());

		// Read calibration after applying the resolution settings.
		status = scGetSensorIntrinsicParameters(m_scDevHandle, SC_TOF_SENSOR, &m_scCamParams);
		cout << "Get scGetSensorIntrinsicParameters status: " << status << endl;
		cout << "ToF Sensor Intinsic: " << endl;
		cout << "Fx: " << m_scCamParams.fx << endl;
		cout << "Cx: " << m_scCamParams.cx << endl;
		cout << "Fy: " << m_scCamParams.fy << endl;
		cout << "Cy: " << m_scCamParams.cy << endl;
		cout << "ToF Sensor Distortion Coefficient: " << endl;
		cout << "K1: " << m_scCamParams.k1 << endl;
		cout << "K2: " << m_scCamParams.k2 << endl;
		cout << "P1: " << m_scCamParams.p1 << endl;
		cout << "P2: " << m_scCamParams.p2 << endl;
		cout << "K3: " << m_scCamParams.k3 << endl;
		cout << "K4: " << m_scCamParams.k4 << endl;
		cout << "K5: " << m_scCamParams.k5 << endl;
		cout << "K6: " << m_scCamParams.k6 << endl;

		const int nBfw = 64;
		char fw[nBfw] = {0};
		scGetFirmwareVersion(m_scDevHandle, fw, nBfw);
		LOG_I("fw  ==  " + string(fw));
		LOG_I("sn  ==  " + string(m_pScDevListInfo[0].serialNumber));

		status = scStartStream(m_scDevHandle);
		if (status != SC_OK)
		{
			LOG_E("StartStream failed: " + i2str(status));
			scCloseDevice(&m_scDevHandle);
			m_scDevHandle = 0;
			return false;
		}

		int width = m_vSizeD.x();
		int height = m_vSizeD.y();
		if (scGetToFResolution(m_scDevHandle, &width, &height) == SC_OK)
			m_vSizeD = Vector2i(width, height);

		m_tFrameInterval = 2 * 1000 / m_scCtrl.m_frameRate;
		m_bOpened = true;
		return true;
	}

	void _Scepter::close(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_bOpened = false;
		m_bPCLframe = false;

		if (m_scDevHandle)
		{
			scStopStream(m_scDevHandle);
			ScStatus status = scCloseDevice(&m_scDevHandle);
			m_scDevHandle = 0;
			LOG_I("CloseDevice status: " + i2str(status));
		}

		delete[] m_pScDevListInfo;
		m_pScDevListInfo = nullptr;
	}

	bool _Scepter::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTpp);

		IF_F(!m_pT->startThread(getUpdate, this));
		return m_pTpp->startThread(getTPP, this);
	}

	bool _Scepter::check(void)
	{
		return this->_RGBDbase::check();
	}

	void _Scepter::update(void)
	{
		ScStatus status = scInitialize();

		if (status != ScStatus::SC_OK)
		{
			LOG_E("ScInitialize failed");
			return;
		}
		m_bScInitialized = true;

		while (m_pT->bRun())
		{
			if (!open())
			{
				LOG_E("Cannot open Scense");
				m_pT->sleepT(NSEC_SEC);
				continue;
			}

			m_pT->autoFPS();

			if (updateScRGBD())
			{
				m_pTpp->run();
			}
		}
	}

	bool _Scepter::updateScRGBD(void)
	{
		std::lock_guard<std::recursive_mutex> frameLock(m_mutexScFrame);
		IF_F(!m_bOpened || !m_scDevHandle);
		// A new capture invalidates the previous SDK buffers, even if it fails.
		m_bPCLframe = false;
		m_scfRGB = {};
		m_scfDepth = {};
		m_scfTransformedRGB = {};
		m_scfTransformedDepth = {};
		m_scfIR = {};
		ScFrameReady sFr = {0};
		ScStatus status = scGetFrameReady(m_scDevHandle,
										  m_tFrameInterval,
										  &sFr);
		// if(status == cam_lost)
		// 	m_bOpened = false;

		IF_F(status != SC_OK);

		if (m_bRGB && sFr.color == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_COLOR_FRAME, &m_scfRGB);
			if (status == SC_OK)
			{
				std::lock_guard<std::mutex> lock(m_mutexRGB);
				if (copyScFrame(m_scfRGB, m_mRGB))
					m_vSizeRGB = Vector2i(m_scfRGB.width, m_scfRGB.height);
			}
		}

		if (m_bDepth && sFr.depth == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_DEPTH_FRAME, &m_scfDepth);
			if (status == SC_OK && m_scfDepth.pixelFormat == SC_PIXEL_FORMAT_DEPTH_MM16)
			{
				std::lock_guard<std::mutex> lock(m_mutexDepth);
				if (copyScFrame(m_scfDepth, m_mDepth))
				{
					m_vSizeD = Vector2i(m_scfDepth.width, m_scfDepth.height);
					m_bPCLframe = true;
				}
			}
		}

		if (m_btRGB && sFr.transformedColor == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_TRANSFORM_COLOR_IMG_TO_DEPTH_SENSOR_FRAME, &m_scfTransformedRGB);
			if (status == SC_OK)
			{
				std::lock_guard<std::mutex> lock(m_mutexRGB);
				if (!copyScFrame(m_scfTransformedRGB, m_mtRGB))
					m_scfTransformedRGB = {};
			}
			else
				m_scfTransformedRGB = {};
		}

		if (m_btDepth && sFr.transformedDepth == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_TRANSFORM_DEPTH_IMG_TO_COLOR_SENSOR_FRAME, &m_scfTransformedDepth);
			if (status == SC_OK && m_scfTransformedDepth.pixelFormat == SC_PIXEL_FORMAT_DEPTH_MM16)
			{
				std::lock_guard<std::mutex> lock(m_mutexDepth);
				copyScFrame(m_scfTransformedDepth, m_mtDepth);
			}
		}

		if (m_bIR && sFr.ir == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_IR_FRAME, &m_scfIR);
			if (status == SC_OK)
			{
				std::lock_guard<std::mutex> lock(m_mutexDepth);
				copyScFrame(m_scfIR, m_mIR);
			}
		}

		return true;
	}

	void _Scepter::updateTPP(void)
	{
		while (m_pTpp->bRun())
		{
			// Bound the wait so a wakeup during processing cannot strand a frame.
			m_pTpp->autoFPS();

			updatePCL();
		}
	}

	void _Scepter::updatePCL(void)
	{
#ifdef WITH_UNIVERSE

		NULL_(m_pPCL);

		vector<ScVector3f> points;
		Mat color;
		int width, height, stride;
		float scale;
		{
			std::lock_guard<std::recursive_mutex> frameLock(m_mutexScFrame);

			IF_(!m_bOpened || !m_scDevHandle || !m_bPCLframe || (!m_bPCL && !m_bPCLrgb));
			stride = m_pclStride; scale = m_dScale;
			m_bPCLframe = false;

			// Use this frame's dimensions; the device may reject a requested resolution.
			width = m_scfDepth.width;
			height = m_scfDepth.height;
			points.resize(size_t(m_scfDepth.width) * m_scfDepth.height);

			IF_(scConvertDepthFrameToPointCloudVector(m_scDevHandle, &m_scfDepth, points.data()) != SC_OK);

			if (m_bPCLrgb && m_scfTransformedRGB.pFrameData &&
				m_scfTransformedRGB.width == m_scfDepth.width &&
				m_scfTransformedRGB.height == m_scfDepth.height)
			{
				std::lock_guard<std::mutex> lock(m_mutexRGB);
				if (m_mtRGB.type() == CV_8UC3)
					color = m_mtRGB.clone();
			}
		}

		constexpr float c_b = 1.0f / 255.0f;
		const uint64_t tNow = getTns();

		m_pPCL->frameStart();
		// Sample in image space so XYZ and aligned color use the same original
		// pixel. Keep SDK conversion at native resolution to preserve calibration.
		for (int y = 0; y < height; y += stride)
		{
			for (int x = 0; x < width; x += stride)
			{
				const size_t k = size_t(y) * width + x;
				const auto &p = points[k];

				// Keep validation and scaling scalar, as in Orbbec. Eigen expression
				// evaluation per pixel is costly in unoptimized camera/debug builds.
				IF_CONT(!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) || p.z <= 0 || p.z == UINT16_MAX);
				const Vector3f vP(p.x * scale, p.y * scale, p.z * scale);
				Vector3f vC(1.f, 1.f, 1.f);
				if (!color.empty())
				{
					const uint8_t *pC = color.ptr<uint8_t>() + k * 3;
					vC = Vector3f(pC[2] * c_b, pC[1] * c_b, pC[0] * c_b);
				}
				m_pPCL->add(vP, vC, tNow);
			}
		}
		m_pPCL->frameStop();
#endif
	}

	void _Scepter::console(const json &j, void *pJSONbase)
	{
		auto *transport = static_cast<_JSONbase *>(pJSONbase);
		if (!transport || !j.is_object() || !j.contains("cmd") || !j["cmd"].is_string()) return;
		const string cmd = j["cmd"].get<string>();
		if (cmd != "getConfig" && cmd != "setConfig" && cmd != "saveConfig") return;
		json reply = {{"cmd", cmd}, {"module", getName()}, {"bSuccess", true}};
		if (j.contains("requestId")) reply["requestId"] = j["requestId"];
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
			try
			{
				if (cmd == "setConfig")
				{
					json errors;
					reply["bSuccess"] = applyConfig(j.value("config", json()), true, errors);
					reply["errors"] = errors;
				}
				else if (cmd == "saveConfig")
				{
					reply["bSuccess"] = saveConfig(true);
					if (!reply["bSuccess"].get<bool>())
					{
						reply["error"] = "Could not save the launch configuration";
					}
				}
				if (cmd == "getConfig") reply["schema"] = controlSchema();
			}
			catch (const std::exception &e) { reply["bSuccess"] = false; reply["error"] = e.what(); }
			reply["config"] = configValues();
			reply["deviceOpen"] = m_bOpened;
		}
		// Sending may block; never hold up capture on the command transport.
		transport->sendJson(reply);
	}

	bool _Scepter::setToFexposureControlMode(bool bAuto)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bAutoExposureToF = bAuto;

		ScStatus ScR = scSetExposureControlMode(m_scDevHandle,
												SC_TOF_SENSOR,
												bAuto ? SC_EXPOSURE_CONTROL_MODE_AUTO
													  : SC_EXPOSURE_CONTROL_MODE_MANUAL);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setToFexposureTime(int tExposure)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_tExposureToF = tExposure;

		ScStatus ScR = scSetExposureTime(m_scDevHandle,
										 SC_TOF_SENSOR,
										 tExposure);

		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setRGBexposureControlMode(bool bAuto)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bAutoExposureRGB = bAuto;

		ScStatus ScR = scSetExposureControlMode(m_scDevHandle,
												SC_COLOR_SENSOR,
												bAuto ? SC_EXPOSURE_CONTROL_MODE_AUTO
													  : SC_EXPOSURE_CONTROL_MODE_MANUAL);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setRGBexposureTime(int tExposure)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_tExposureRGB = tExposure;

		ScStatus ScR = scSetExposureTime(m_scDevHandle,
										 SC_COLOR_SENSOR,
										 tExposure);

		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setTimeFilter(bool bON, int thr)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_timeFilter = {thr, bON};

		ScStatus ScR = scSetTimeFilterParams(m_scDevHandle, m_scCtrl.m_timeFilter);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setConfidenceFilter(bool bON, int thr)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_confidenceFilter = {thr, bON};

		ScStatus ScR = scSetConfidenceFilterParams(m_scDevHandle, m_scCtrl.m_confidenceFilter);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFlyingPixelFilter(bool bON, int thr)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_flyingPixelFilter = {thr, bON};

		ScStatus ScR = scSetFlyingPixelFilterParams(m_scDevHandle, m_scCtrl.m_flyingPixelFilter);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFillHole(bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bFillHole = bON;

		ScStatus ScR = scSetFillHoleFilterEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setSpatialFilter(bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bSpatialFilter = bON;

		ScStatus ScR = scSetSpatialFilterEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setHDR(bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bHDR = bON;

		ScStatus ScR = scSetHDRModeEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFrameRate(int fps)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		IF_F(fps <= 0);
		m_scCtrl.m_frameRate = fps;

		ScStatus ScR = scSetFrameRate(m_scDevHandle, fps);
		if (ScR == SC_OK)
			m_tFrameInterval = 2 * 1000 / fps;
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setWorkMode(ScWorkMode mode)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_workMode = mode;

		ScStatus ScR = scSetWorkMode(m_scDevHandle, mode);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setSoftwareTriggerParameter(uint8_t frameCount)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_softwareTriggerFrameCount = frameCount;

		ScStatus ScR = scSetSoftwareTriggerParameter(m_scDevHandle, frameCount);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setInputSignalParamsForHWTrigger(uint32_t width, uint32_t interval, uint8_t polarity)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_hwTrigger = {width, interval, polarity};

		ScStatus ScR = scSetInputSignalParamsForHWTrigger(m_scDevHandle, m_scCtrl.m_hwTrigger);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setToFResolution(int width, int height)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		IF_F(width <= 0 || width > UINT16_MAX || height <= 0 || height > UINT16_MAX);
		m_scCtrl.m_resolutionToF = {(uint16_t)width, (uint16_t)height};

		ScStatus ScR = scSetToFResolution(m_scDevHandle, width, height);
		if (ScR == SC_OK)
			m_vSizeD = Vector2i(width, height);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorResolution(int width, int height)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		IF_F(width <= 0 || width > UINT16_MAX || height <= 0 || height > UINT16_MAX);
		m_scCtrl.m_resolutionRGB = {(uint16_t)width, (uint16_t)height};

		ScStatus ScR = scSetColorResolution(m_scDevHandle, width, height);
		if (ScR == SC_OK)
			m_vSizeRGB = Vector2i(width, height);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorPixelFormat(ScPixelFormat pixelFormat)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_pixelFormat = pixelFormat;

		ScStatus ScR = scSetColorPixelFormat(m_scDevHandle, pixelFormat);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setTransformColorImgToDepthSensorEnabled(bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bTransformRGBToDepth = bON;

		ScStatus ScR = scSetTransformColorImgToDepthSensorEnabled(m_scDevHandle, bON);
		if (ScR == SC_OK)
			m_btRGB = bON;
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setTransformDepthImgToColorSensorEnabled(bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bTransformDepthToRGB = bON;

		ScStatus ScR = scSetTransformDepthImgToColorSensorEnabled(m_scDevHandle, bON);
		if (ScR == SC_OK)
			m_btDepth = bON;
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setIRGMMGain(uint8_t gain)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_irGMMGain = gain;

		ScStatus ScR = scSetIRGMMGain(m_scDevHandle, gain);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setIRGMMCorrection(bool bON, int thr)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_irGMMCorrection = {thr, bON};

		ScStatus ScR = scSetIRGMMCorrection(m_scDevHandle, m_scCtrl.m_irGMMCorrection);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorGain(float gain)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_gainRGB = gain;

		ScStatus ScR = scSetColorGain(m_scDevHandle, gain);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorAECMaxExposureTime(int tExposure)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_tAECMaxExposureRGB = tExposure;

		ScStatus ScR = scSetColorAECMaxExposureTime(m_scDevHandle, tExposure);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorAECROI(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_aecROIRGBorigin = {x, y};
		m_scCtrl.m_aecROIRGBsize = {width, height};

		ScStatus ScR = scSetColorAECROI(m_scDevHandle, x, y, width, height);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setExposureTimeOfHDR(uint8_t frameIndex, int tExposure)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_tExposureHDR[frameIndex] = tExposure;

		ScStatus ScR = scSetExposureTimeOfHDR(m_scDevHandle, frameIndex, tExposure);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setWDR(bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bWDR = bON;

		ScStatus ScR = scSetWDRModeEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setExposureTimeOfWDR(uint8_t frameIndex, int tExposure)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_tExposureWDR[frameIndex] = tExposure;

		ScStatus ScR = scSetExposureTimeOfWDR(m_scDevHandle, frameIndex, tExposure);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setDeviceDHCPEnabled(bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bDHCP = bON;

		ScStatus ScR = scSetDeviceDHCPEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setDeviceIPAddr(const string &ipAddr)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		IF_F(ipAddr.empty() || ipAddr.size() >= 16);
		m_scCtrl.m_deviceIPAddr = ipAddr;

		ScStatus ScR = scSetDeviceIPAddr(m_scDevHandle, ipAddr.c_str(), (int32_t)ipAddr.size());
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setDeviceSubnetMask(const string &mask)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		IF_F(mask.empty() || mask.size() >= 16);
		m_scCtrl.m_deviceSubnetMask = mask;

		ScStatus ScR = scSetDeviceSubnetMask(m_scDevHandle, mask.c_str(), (int32_t)mask.size());
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setRealTimeSyncConfig(const ScTimeSyncConfig &params)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_timeSync = params;

		ScStatus ScR = scSetRealTimeSyncConfig(m_scDevHandle, params);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setParamsByJson(const string &filePath)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_paramsJsonFile = filePath;

		ScStatus ScR = scSetParamsByJson(m_scDevHandle, m_scCtrl.m_paramsJsonFile.data());
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setHotPlugStatusCallback(PtrHotPlugStatusCallback pCallback, const void *pUserData)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_hotPlugCallback = pCallback;
		m_scCtrl.m_pHotPlugUserData = pUserData;

		ScStatus ScR = scSetHotPlugStatusCallback(pCallback, pUserData);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModuleEnabled(bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_bAIModule = bON;

		ScStatus ScR = scAIModuleSetEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModuleWorkMode(ScAIModuleMode mode)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_aiWorkMode = mode;

		ScStatus ScR = scAIModuleSetWorkMode(m_scDevHandle, mode);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModuleParam(uint32_t paramID, const std::vector<uint8_t> &data)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		IF_F(data.size() > UINT16_MAX);
		m_scCtrl.m_aiParams[paramID] = data;

		std::vector<uint8_t> &param = m_scCtrl.m_aiParams[paramID];
		ScStatus ScR = scAIModuleSetParam(m_scDevHandle, paramID, param.data(), (uint16_t)param.size());
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModuleInputFrameTypeEnabled(ScFrameType frameType, bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_aiInputFrames[frameType] = bON;

		ScStatus ScR = scAIModuleSetInputFrameTypeEnabled(m_scDevHandle, frameType, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModulePreviewFrameTypeEnabled(ScFrameType frameType, bool bON)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutexScFrame);
		m_scCtrl.m_aiPreviewFrames[frameType] = bON;

		ScStatus ScR = scAIModuleSetPreviewFrameTypeEnabled(m_scDevHandle, frameType, bON);
		return (ScR == SC_OK) ? true : false;
	}
}
