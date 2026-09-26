/*
 * _Orbbec.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_Orbbec.h"
#include <stdexcept>
#include <type_traits>

// Reject coercions, truncation and overflow before any device write.
template <typename T> static T readControl(const json &j)
{
	if constexpr (std::is_same_v<T, bool>)
	{
		if (!j.is_boolean()) throw std::invalid_argument("Expected a boolean");
	}
	else if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
	{
		if (!j.is_number_integer()) throw std::invalid_argument("Expected an integer");
		const long double n = j.get<long double>();
		if constexpr (std::is_enum_v<T>)
		{
			using U = std::underlying_type_t<T>;
			if (n < std::numeric_limits<U>::lowest() || n > std::numeric_limits<U>::max()) throw std::invalid_argument("Integer out of range");
		}
		else if (n < std::numeric_limits<T>::lowest() || n > std::numeric_limits<T>::max()) throw std::invalid_argument("Integer out of range");
	}
	else if constexpr (std::is_floating_point_v<T>)
	{
		if (!j.is_number() || !std::isfinite(j.get<T>())) throw std::invalid_argument("Expected a finite number");
	}
	else if constexpr (std::is_same_v<T, string>)
	{
		if (!j.is_string()) throw std::invalid_argument("Expected a string");
	}
	return j.get<T>();
}

template <typename T> static bool requiredControl(const json &j, const string &key, T &value)
{
	if (!j.contains(key) || j[key].is_null()) return false;
	value = readControl<T>(j[key]);
	return true;
}


// SDK C structures do not provide JSON conversions. Require complete objects to
// avoid writing zero-filled, partially configured exposure or network settings.
static void from_json(const json &j, OBMultiDeviceSyncConfig &value)
{
	value = {};
	if (!requiredControl(j, "syncMode", value.syncMode))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.syncMode");
	if (!requiredControl(j, "depthDelayUs", value.depthDelayUs))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.depthDelayUs");
	if (!requiredControl(j, "colorDelayUs", value.colorDelayUs))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.colorDelayUs");
	if (!requiredControl(j, "trigger2ImageDelayUs", value.trigger2ImageDelayUs))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.trigger2ImageDelayUs");
	if (!requiredControl(j, "triggerOutEnable", value.triggerOutEnable))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.triggerOutEnable");
	if (!requiredControl(j, "triggerOutDelayUs", value.triggerOutDelayUs))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.triggerOutDelayUs");
	if (!requiredControl(j, "framesPerTrigger", value.framesPerTrigger))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.framesPerTrigger");
}

static void from_json(const json &j, OBNetIpConfig &value)
{
	value = {};
	if (!requiredControl(j, "dhcp", value.dhcp))
		throw std::invalid_argument("Missing or invalid OBNetIpConfig.dhcp");
	{
		json octets;
		if (!requiredControl(j, "address", octets) || !octets.is_array() || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfig.address requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (readControl<int>(octets[i]) < 0 || octets[i].get<int>() > 255)
				throw std::invalid_argument("Invalid IPv4 octet in address");
			value.address[i] = octets[i].get<uint8_t>();
		}
	}
	{
		json octets;
		if (!requiredControl(j, "mask", octets) || !octets.is_array() || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfig.mask requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (readControl<int>(octets[i]) < 0 || octets[i].get<int>() > 255)
				throw std::invalid_argument("Invalid IPv4 octet in mask");
			value.mask[i] = octets[i].get<uint8_t>();
		}
	}
	{
		json octets;
		if (!requiredControl(j, "gateway", octets) || !octets.is_array() || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfig.gateway requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (readControl<int>(octets[i]) < 0 || octets[i].get<int>() > 255)
				throw std::invalid_argument("Invalid IPv4 octet in gateway");
			value.gateway[i] = octets[i].get<uint8_t>();
		}
	}
}

static void from_json(const json &j, OBHdrConfig &value)
{
	value = {};
	if (!requiredControl(j, "enable", value.enable))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.enable");
	if (!requiredControl(j, "sequence_name", value.sequence_name))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.sequence_name");
	if (!requiredControl(j, "exposure_1", value.exposure_1))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.exposure_1");
	if (!requiredControl(j, "gain_1", value.gain_1))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.gain_1");
	if (!requiredControl(j, "exposure_2", value.exposure_2))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.exposure_2");
	if (!requiredControl(j, "gain_2", value.gain_2))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.gain_2");
}

static void from_json(const json &j, OBRegionOfInterest &value)
{
	value = {};
	if (!requiredControl(j, "x0_left", value.x0_left))
		throw std::invalid_argument("Missing or invalid OBRegionOfInterest.x0_left");
	if (!requiredControl(j, "y0_top", value.y0_top))
		throw std::invalid_argument("Missing or invalid OBRegionOfInterest.y0_top");
	if (!requiredControl(j, "x1_right", value.x1_right))
		throw std::invalid_argument("Missing or invalid OBRegionOfInterest.x1_right");
	if (!requiredControl(j, "y1_bottom", value.y1_bottom))
		throw std::invalid_argument("Missing or invalid OBRegionOfInterest.y1_bottom");
}

static void from_json(const json &j, OBDispOffsetConfig &value)
{
	value = {};
	if (!requiredControl(j, "enable", value.enable))
		throw std::invalid_argument("Missing or invalid OBDispOffsetConfig.enable");
	if (!requiredControl(j, "offset0", value.offset0))
		throw std::invalid_argument("Missing or invalid OBDispOffsetConfig.offset0");
	if (!requiredControl(j, "offset1", value.offset1))
		throw std::invalid_argument("Missing or invalid OBDispOffsetConfig.offset1");
}

static void from_json(const json &j, OBPresetResolutionConfig &value)
{
	value = {};
	if (!requiredControl(j, "width", value.width))
		throw std::invalid_argument("Missing or invalid OBPresetResolutionConfig.width");
	if (!requiredControl(j, "height", value.height))
		throw std::invalid_argument("Missing or invalid OBPresetResolutionConfig.height");
	if (!requiredControl(j, "irDecimationFactor", value.irDecimationFactor))
		throw std::invalid_argument("Missing or invalid OBPresetResolutionConfig.irDecimationFactor");
	if (!requiredControl(j, "depthDecimationFactor", value.depthDecimationFactor))
		throw std::invalid_argument("Missing or invalid OBPresetResolutionConfig.depthDecimationFactor");
}

static void from_json(const json &j, OBNetIpConfigV2 &value)
{
	value = {};
	if (!requiredControl(j, "flags", value.flags))
		throw std::invalid_argument("Missing or invalid OBNetIpConfigV2.flags");
	{
		json octets;
		if (!requiredControl(j, "address", octets) || !octets.is_array() || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfigV2.address requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (readControl<int>(octets[i]) < 0 || octets[i].get<int>() > 255)
				throw std::invalid_argument("Invalid IPv4 octet in address");
			value.address[i] = octets[i].get<uint8_t>();
		}
	}
	{
		json octets;
		if (!requiredControl(j, "mask", octets) || !octets.is_array() || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfigV2.mask requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (readControl<int>(octets[i]) < 0 || octets[i].get<int>() > 255)
				throw std::invalid_argument("Invalid IPv4 octet in mask");
			value.mask[i] = octets[i].get<uint8_t>();
		}
	}
	{
		json octets;
		if (!requiredControl(j, "gateway", octets) || !octets.is_array() || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfigV2.gateway requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (readControl<int>(octets[i]) < 0 || octets[i].get<int>() > 255)
				throw std::invalid_argument("Invalid IPv4 octet in gateway");
			value.gateway[i] = octets[i].get<uint8_t>();
		}
	}
}

static void from_json(const json &j, OBDeviceTimestampResetConfig &value)
{
	value = {};
	if (!requiredControl(j, "enable", value.enable))
		throw std::invalid_argument("Missing or invalid OBDeviceTimestampResetConfig.enable");
	if (!requiredControl(j, "timestamp_reset_delay_us", value.timestamp_reset_delay_us))
		throw std::invalid_argument("Missing or invalid OBDeviceTimestampResetConfig.timestamp_reset_delay_us");
	if (!requiredControl(j, "timestamp_reset_signal_output_enable", value.timestamp_reset_signal_output_enable))
		throw std::invalid_argument("Missing or invalid OBDeviceTimestampResetConfig.timestamp_reset_signal_output_enable");
}

static void to_json(json &j, const OBMultiDeviceSyncConfig &value)
{
	j = json::object();
	j["syncMode"] = value.syncMode;
	j["depthDelayUs"] = value.depthDelayUs;
	j["colorDelayUs"] = value.colorDelayUs;
	j["trigger2ImageDelayUs"] = value.trigger2ImageDelayUs;
	j["triggerOutEnable"] = value.triggerOutEnable;
	j["triggerOutDelayUs"] = value.triggerOutDelayUs;
	j["framesPerTrigger"] = value.framesPerTrigger;
}

static void to_json(json &j, const OBNetIpConfig &value)
{
	j = json::object();
	j["dhcp"] = value.dhcp;
	j["address"] = value.address;
	j["mask"] = value.mask;
	j["gateway"] = value.gateway;
}

static void to_json(json &j, const OBHdrConfig &value)
{
	j = json::object();
	j["enable"] = value.enable;
	j["sequence_name"] = value.sequence_name;
	j["exposure_1"] = value.exposure_1;
	j["gain_1"] = value.gain_1;
	j["exposure_2"] = value.exposure_2;
	j["gain_2"] = value.gain_2;
}

static void to_json(json &j, const OBRegionOfInterest &value)
{
	j = json::object();
	j["x0_left"] = value.x0_left;
	j["y0_top"] = value.y0_top;
	j["x1_right"] = value.x1_right;
	j["y1_bottom"] = value.y1_bottom;
}

static void to_json(json &j, const OBDispOffsetConfig &value)
{
	j = json::object();
	j["enable"] = value.enable;
	j["offset0"] = value.offset0;
	j["offset1"] = value.offset1;
}

static void to_json(json &j, const OBPresetResolutionConfig &value)
{
	j = json::object();
	j["width"] = value.width;
	j["height"] = value.height;
	j["irDecimationFactor"] = value.irDecimationFactor;
	j["depthDecimationFactor"] = value.depthDecimationFactor;
}

static void to_json(json &j, const OBNetIpConfigV2 &value)
{
	j = json::object();
	j["flags"] = value.flags;
	j["address"] = value.address;
	j["mask"] = value.mask;
	j["gateway"] = value.gateway;
}

static void to_json(json &j, const OBDeviceTimestampResetConfig &value)
{
	j = json::object();
	j["enable"] = value.enable;
	j["timestamp_reset_delay_us"] = value.timestamp_reset_delay_us;
	j["timestamp_reset_signal_output_enable"] = value.timestamp_reset_signal_output_enable;
}

namespace kai
{

	_Orbbec::_Orbbec()
	{
		m_vRangeD = {0.0f, 5.0f};
		m_dScale = 0.001f;
	}

	_Orbbec::~_Orbbec()
	{
		stop();
	}

	bool _Orbbec::loadConfig(void)
	{
		if (!_RGBDbase::loadConfig())
		{
			return false;
		}
		const json &j = *m_pJ;

		json startup = json::object();
		const json controls = configValues();
		for (const auto &field : controls.items())
		{
			auto it = j.find(field.key());
			if (it != j.end())
			{
				startup[field.key()] = *it;
			}
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

	bool _Orbbec::link(void)
	{
		if (!_RGBDbase::link() || !m_pTpp || !m_pTpp->link())
		{
			return false;
		}

		return true;
	}

	json _Orbbec::configValues(void) const
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		json j = {
			{"SN", m_SN}, {"bNetDevEnum", m_bNetDevEnum}, {"tOutMs", m_tOutMs},
			{"bRGB", m_bRGB}, {"bDepth", m_bDepth}, {"bIMU", m_bIMU},
			{"bPCL", m_bPCL}, {"bPCLrgb", m_bPCLrgb},
			{"devFPS", m_devFPS}, {"devFPSd", m_devFPSd},
			{"vSizeRGB", {m_vSizeRGB.x(), m_vSizeRGB.y()}},
			{"vSizeD", {m_vSizeD.x(), m_vSizeD.y()}}, {"dScale", m_dScale}};
		j["OB_PROP_LDP_BOOL"] = m_orbbecCtrl.m_ldp ? json(*m_orbbecCtrl.m_ldp) : json(nullptr);
		j["OB_PROP_LASER_BOOL"] = m_orbbecCtrl.m_laser ? json(*m_orbbecCtrl.m_laser) : json(nullptr);
		j["OB_PROP_FLOOD_BOOL"] = m_orbbecCtrl.m_flood ? json(*m_orbbecCtrl.m_flood) : json(nullptr);
		j["OB_PROP_TEMPERATURE_COMPENSATION_BOOL"] = m_orbbecCtrl.m_temperatureCompensation ? json(*m_orbbecCtrl.m_temperatureCompensation) : json(nullptr);
		j["OB_PROP_DEPTH_MIRROR_BOOL"] = m_orbbecCtrl.m_depthMirror ? json(*m_orbbecCtrl.m_depthMirror) : json(nullptr);
		j["OB_PROP_DEPTH_FLIP_BOOL"] = m_orbbecCtrl.m_depthFlip ? json(*m_orbbecCtrl.m_depthFlip) : json(nullptr);
		j["OB_PROP_DEPTH_HOLEFILTER_BOOL"] = m_orbbecCtrl.m_depthHolefilter ? json(*m_orbbecCtrl.m_depthHolefilter) : json(nullptr);
		j["OB_PROP_IR_MIRROR_BOOL"] = m_orbbecCtrl.m_irMirror ? json(*m_orbbecCtrl.m_irMirror) : json(nullptr);
		j["OB_PROP_IR_FLIP_BOOL"] = m_orbbecCtrl.m_irFlip ? json(*m_orbbecCtrl.m_irFlip) : json(nullptr);
		j["OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL"] = m_orbbecCtrl.m_depthNoiseRemovalFilter ? json(*m_orbbecCtrl.m_depthNoiseRemovalFilter) : json(nullptr);
		j["OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT"] = m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff ? json(*m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff) : json(nullptr);
		j["OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT"] = m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize ? json(*m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize) : json(nullptr);
		j["OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL"] = m_orbbecCtrl.m_depthAlignHardware ? json(*m_orbbecCtrl.m_depthAlignHardware) : json(nullptr);
		j["OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT"] = m_orbbecCtrl.m_depthAlignHardwareMode ? json(*m_orbbecCtrl.m_depthAlignHardwareMode) : json(nullptr);
		j["OB_PROP_DEPTH_PRECISION_LEVEL_INT"] = m_orbbecCtrl.m_depthPrecisionLevel ? json(*m_orbbecCtrl.m_depthPrecisionLevel) : json(nullptr);
		j["OB_PROP_COLOR_MIRROR_BOOL"] = m_orbbecCtrl.m_colorMirror ? json(*m_orbbecCtrl.m_colorMirror) : json(nullptr);
		j["OB_PROP_COLOR_FLIP_BOOL"] = m_orbbecCtrl.m_colorFlip ? json(*m_orbbecCtrl.m_colorFlip) : json(nullptr);
		j["OB_PROP_DISPARITY_TO_DEPTH_BOOL"] = m_orbbecCtrl.m_disparityToDepth ? json(*m_orbbecCtrl.m_disparityToDepth) : json(nullptr);
		j["OB_PROP_WATCHDOG_BOOL"] = m_orbbecCtrl.m_watchdog ? json(*m_orbbecCtrl.m_watchdog) : json(nullptr);
		j["OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL"] = m_orbbecCtrl.m_externalSignalReset ? json(*m_orbbecCtrl.m_externalSignalReset) : json(nullptr);
		j["OB_PROP_HEARTBEAT_BOOL"] = m_orbbecCtrl.m_heartbeat ? json(*m_orbbecCtrl.m_heartbeat) : json(nullptr);
		j["OB_PROP_LASER_POWER_LEVEL_CONTROL_INT"] = m_orbbecCtrl.m_laserPowerLevelControl ? json(*m_orbbecCtrl.m_laserPowerLevelControl) : json(nullptr);
		j["OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL"] = m_orbbecCtrl.m_timerResetTriggerOutEnable ? json(*m_orbbecCtrl.m_timerResetTriggerOutEnable) : json(nullptr);
		j["OB_PROP_TIMER_RESET_DELAY_US_INT"] = m_orbbecCtrl.m_timerResetDelayUs ? json(*m_orbbecCtrl.m_timerResetDelayUs) : json(nullptr);
		j["OB_PROP_IR_RIGHT_MIRROR_BOOL"] = m_orbbecCtrl.m_irRightMirror ? json(*m_orbbecCtrl.m_irRightMirror) : json(nullptr);
		j["OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT"] = m_orbbecCtrl.m_captureImageFrameNumber ? json(*m_orbbecCtrl.m_captureImageFrameNumber) : json(nullptr);
		j["OB_PROP_IR_RIGHT_FLIP_BOOL"] = m_orbbecCtrl.m_irRightFlip ? json(*m_orbbecCtrl.m_irRightFlip) : json(nullptr);
		j["OB_PROP_COLOR_ROTATE_INT"] = m_orbbecCtrl.m_colorRotate ? json(*m_orbbecCtrl.m_colorRotate) : json(nullptr);
		j["OB_PROP_IR_ROTATE_INT"] = m_orbbecCtrl.m_irRotate ? json(*m_orbbecCtrl.m_irRotate) : json(nullptr);
		j["OB_PROP_IR_RIGHT_ROTATE_INT"] = m_orbbecCtrl.m_irRightRotate ? json(*m_orbbecCtrl.m_irRightRotate) : json(nullptr);
		j["OB_PROP_DEPTH_ROTATE_INT"] = m_orbbecCtrl.m_depthRotate ? json(*m_orbbecCtrl.m_depthRotate) : json(nullptr);
		j["OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL"] = m_orbbecCtrl.m_syncSignalTriggerOut ? json(*m_orbbecCtrl.m_syncSignalTriggerOut) : json(nullptr);
		j["OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL"] = m_orbbecCtrl.m_deviceUSB2RepeatIdentify ? json(*m_orbbecCtrl.m_deviceUSB2RepeatIdentify) : json(nullptr);
		j["OB_PROP_LASER_ALWAYS_ON_BOOL"] = m_orbbecCtrl.m_laserAlwaysOn ? json(*m_orbbecCtrl.m_laserAlwaysOn) : json(nullptr);
		j["OB_PROP_LASER_ON_OFF_PATTERN_INT"] = m_orbbecCtrl.m_laserOnOffPattern ? json(*m_orbbecCtrl.m_laserOnOffPattern) : json(nullptr);
		j["OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT"] = m_orbbecCtrl.m_depthUnitFlexibleAdjustment ? json(*m_orbbecCtrl.m_depthUnitFlexibleAdjustment) : json(nullptr);
		j["OB_PROP_LASER_CONTROL_INT"] = m_orbbecCtrl.m_laserControl ? json(*m_orbbecCtrl.m_laserControl) : json(nullptr);
		j["OB_PROP_IR_BRIGHTNESS_INT"] = m_orbbecCtrl.m_irBrightness ? json(*m_orbbecCtrl.m_irBrightness) : json(nullptr);
		j["OB_PROP_COLOR_AE_MAX_EXPOSURE_INT"] = m_orbbecCtrl.m_colorAEMaxExposure ? json(*m_orbbecCtrl.m_colorAEMaxExposure) : json(nullptr);
		j["OB_PROP_IR_AE_MAX_EXPOSURE_INT"] = m_orbbecCtrl.m_irAEMaxExposure ? json(*m_orbbecCtrl.m_irAEMaxExposure) : json(nullptr);
		j["OB_PROP_DISP_SEARCH_RANGE_MODE_INT"] = m_orbbecCtrl.m_dispSearchRangeMode ? json(*m_orbbecCtrl.m_dispSearchRangeMode) : json(nullptr);
		j["OB_PROP_DISP_SEARCH_OFFSET_INT"] = m_orbbecCtrl.m_dispSearchOffset ? json(*m_orbbecCtrl.m_dispSearchOffset) : json(nullptr);
		j["OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL"] = m_orbbecCtrl.m_cpuTemperatureCalibration ? json(*m_orbbecCtrl.m_cpuTemperatureCalibration) : json(nullptr);
		j["OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT"] = m_orbbecCtrl.m_frameInterleaveConfigIndex ? json(*m_orbbecCtrl.m_frameInterleaveConfigIndex) : json(nullptr);
		j["OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL"] = m_orbbecCtrl.m_frameInterleaveEnable ? json(*m_orbbecCtrl.m_frameInterleaveEnable) : json(nullptr);
		j["OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT"] = m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay ? json(*m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay) : json(nullptr);
		j["OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL"] = m_orbbecCtrl.m_onChipCalibrationEnable ? json(*m_orbbecCtrl.m_onChipCalibrationEnable) : json(nullptr);
		j["OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL"] = m_orbbecCtrl.m_hwNoiseRemoveFilterEnable ? json(*m_orbbecCtrl.m_hwNoiseRemoveFilterEnable) : json(nullptr);
		j["OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT"] = m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold ? json(*m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold) : json(nullptr);
		j["OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL"] = m_orbbecCtrl.m_deviceAutoCaptureEnable ? json(*m_orbbecCtrl.m_deviceAutoCaptureEnable) : json(nullptr);
		j["OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT"] = m_orbbecCtrl.m_deviceAutoCaptureIntervalTime ? json(*m_orbbecCtrl.m_deviceAutoCaptureIntervalTime) : json(nullptr);
		j["OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL"] = m_orbbecCtrl.m_devicePTPClockSyncEnable ? json(*m_orbbecCtrl.m_devicePTPClockSyncEnable) : json(nullptr);
		j["OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL"] = m_orbbecCtrl.m_confidenceStreamFilter ? json(*m_orbbecCtrl.m_confidenceStreamFilter) : json(nullptr);
		j["OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT"] = m_orbbecCtrl.m_confidenceStreamFilterThreshold ? json(*m_orbbecCtrl.m_confidenceStreamFilterThreshold) : json(nullptr);
		j["OB_PROP_CONFIDENCE_MIRROR_BOOL"] = m_orbbecCtrl.m_confidenceMirror ? json(*m_orbbecCtrl.m_confidenceMirror) : json(nullptr);
		j["OB_PROP_CONFIDENCE_FLIP_BOOL"] = m_orbbecCtrl.m_confidenceFlip ? json(*m_orbbecCtrl.m_confidenceFlip) : json(nullptr);
		j["OB_PROP_CONFIDENCE_ROTATE_INT"] = m_orbbecCtrl.m_confidenceRotate ? json(*m_orbbecCtrl.m_confidenceRotate) : json(nullptr);
		j["OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT"] = m_orbbecCtrl.m_intraCameraSyncReference ? json(*m_orbbecCtrl.m_intraCameraSyncReference) : json(nullptr);
		j["OB_PROP_COLOR_RIGHT_ROTATE_INT"] = m_orbbecCtrl.m_colorRightRotate ? json(*m_orbbecCtrl.m_colorRightRotate) : json(nullptr);
		j["OB_PROP_COLOR_RIGHT_MIRROR_BOOL"] = m_orbbecCtrl.m_colorRightMirror ? json(*m_orbbecCtrl.m_colorRightMirror) : json(nullptr);
		j["OB_PROP_COLOR_RIGHT_FLIP_BOOL"] = m_orbbecCtrl.m_colorRightFlip ? json(*m_orbbecCtrl.m_colorRightFlip) : json(nullptr);
		j["OB_PROP_DEVICE_AE_REFERENCE_INT"] = m_orbbecCtrl.m_deviceAEReference ? json(*m_orbbecCtrl.m_deviceAEReference) : json(nullptr);
		j["OB_PROP_DEVICE_AE_STRATEGY_INT"] = m_orbbecCtrl.m_deviceAEStrategy ? json(*m_orbbecCtrl.m_deviceAEStrategy) : json(nullptr);
		j["OB_PROP_COLOR_ROI_BRIGHTNESS_INT"] = m_orbbecCtrl.m_colorROIBrightness ? json(*m_orbbecCtrl.m_colorROIBrightness) : json(nullptr);
		j["OB_PROP_COLOR_LEFT_ROTATE_INT"] = m_orbbecCtrl.m_colorLeftRotate ? json(*m_orbbecCtrl.m_colorLeftRotate) : json(nullptr);
		j["OB_PROP_COLOR_LEFT_MIRROR_BOOL"] = m_orbbecCtrl.m_colorLeftMirror ? json(*m_orbbecCtrl.m_colorLeftMirror) : json(nullptr);
		j["OB_PROP_COLOR_LEFT_FLIP_BOOL"] = m_orbbecCtrl.m_colorLeftFlip ? json(*m_orbbecCtrl.m_colorLeftFlip) : json(nullptr);
		j["OB_PROP_COLOR_PRESET_PRIORITY_INT"] = m_orbbecCtrl.m_colorPresetPriority ? json(*m_orbbecCtrl.m_colorPresetPriority) : json(nullptr);
		j["OB_PROP_DEVICE_NETWORK_LLA_BOOL"] = m_orbbecCtrl.m_deviceNetworkLLA ? json(*m_orbbecCtrl.m_deviceNetworkLLA) : json(nullptr);
		j["OB_PROP_COLOR_ANTI_FLICKER_BOOL"] = m_orbbecCtrl.m_colorAntiFlicker ? json(*m_orbbecCtrl.m_colorAntiFlicker) : json(nullptr);
		j["OB_PROP_DEVICE_IP_MODE_INT"] = m_orbbecCtrl.m_deviceIPMode ? json(*m_orbbecCtrl.m_deviceIPMode) : json(nullptr);
		j["OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT"] = m_orbbecCtrl.m_dhcpAssignIPTimeout ? json(*m_orbbecCtrl.m_dhcpAssignIPTimeout) : json(nullptr);
		j["OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT"] = m_orbbecCtrl.m_usbSyncVoltageLevel ? json(*m_orbbecCtrl.m_usbSyncVoltageLevel) : json(nullptr);
		j["OB_PROP_FPS_BOOST_BOOL"] = m_orbbecCtrl.m_fpsBoost ? json(*m_orbbecCtrl.m_fpsBoost) : json(nullptr);
		j["OB_PROP_MJPEG_QUALITY_INT"] = m_orbbecCtrl.m_mjpegQuality ? json(*m_orbbecCtrl.m_mjpegQuality) : json(nullptr);
		j["OB_PROP_COLOR_AUTO_EXPOSURE_BOOL"] = m_orbbecCtrl.m_colorAutoExposure ? json(*m_orbbecCtrl.m_colorAutoExposure) : json(nullptr);
		j["OB_PROP_COLOR_EXPOSURE_INT"] = m_orbbecCtrl.m_colorExposure ? json(*m_orbbecCtrl.m_colorExposure) : json(nullptr);
		j["OB_PROP_COLOR_GAIN_INT"] = m_orbbecCtrl.m_colorGain ? json(*m_orbbecCtrl.m_colorGain) : json(nullptr);
		j["OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL"] = m_orbbecCtrl.m_colorAutoWhiteBalance ? json(*m_orbbecCtrl.m_colorAutoWhiteBalance) : json(nullptr);
		j["OB_PROP_COLOR_WHITE_BALANCE_INT"] = m_orbbecCtrl.m_colorWhiteBalance ? json(*m_orbbecCtrl.m_colorWhiteBalance) : json(nullptr);
		j["OB_PROP_COLOR_BRIGHTNESS_INT"] = m_orbbecCtrl.m_colorBrightness ? json(*m_orbbecCtrl.m_colorBrightness) : json(nullptr);
		j["OB_PROP_COLOR_SHARPNESS_INT"] = m_orbbecCtrl.m_colorSharpness ? json(*m_orbbecCtrl.m_colorSharpness) : json(nullptr);
		j["OB_PROP_COLOR_SATURATION_INT"] = m_orbbecCtrl.m_colorSaturation ? json(*m_orbbecCtrl.m_colorSaturation) : json(nullptr);
		j["OB_PROP_COLOR_CONTRAST_INT"] = m_orbbecCtrl.m_colorContrast ? json(*m_orbbecCtrl.m_colorContrast) : json(nullptr);
		j["OB_PROP_COLOR_GAMMA_INT"] = m_orbbecCtrl.m_colorGamma ? json(*m_orbbecCtrl.m_colorGamma) : json(nullptr);
		j["OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT"] = m_orbbecCtrl.m_colorAutoExposurePriority ? json(*m_orbbecCtrl.m_colorAutoExposurePriority) : json(nullptr);
		j["OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT"] = m_orbbecCtrl.m_colorBacklightCompensation ? json(*m_orbbecCtrl.m_colorBacklightCompensation) : json(nullptr);
		j["OB_PROP_COLOR_HUE_INT"] = m_orbbecCtrl.m_colorHue ? json(*m_orbbecCtrl.m_colorHue) : json(nullptr);
		j["OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT"] = m_orbbecCtrl.m_colorPowerLineFrequency ? json(*m_orbbecCtrl.m_colorPowerLineFrequency) : json(nullptr);
		j["OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL"] = m_orbbecCtrl.m_depthAutoExposure ? json(*m_orbbecCtrl.m_depthAutoExposure) : json(nullptr);
		j["OB_PROP_DEPTH_EXPOSURE_INT"] = m_orbbecCtrl.m_depthExposure ? json(*m_orbbecCtrl.m_depthExposure) : json(nullptr);
		j["OB_PROP_DEPTH_GAIN_INT"] = m_orbbecCtrl.m_depthGain ? json(*m_orbbecCtrl.m_depthGain) : json(nullptr);
		j["OB_PROP_IR_AUTO_EXPOSURE_BOOL"] = m_orbbecCtrl.m_irAutoExposure ? json(*m_orbbecCtrl.m_irAutoExposure) : json(nullptr);
		j["OB_PROP_IR_EXPOSURE_INT"] = m_orbbecCtrl.m_irExposure ? json(*m_orbbecCtrl.m_irExposure) : json(nullptr);
		j["OB_PROP_IR_GAIN_INT"] = m_orbbecCtrl.m_irGain ? json(*m_orbbecCtrl.m_irGain) : json(nullptr);
		j["OB_PROP_IR_CHANNEL_DATA_SOURCE_INT"] = m_orbbecCtrl.m_irChannelDataSource ? json(*m_orbbecCtrl.m_irChannelDataSource) : json(nullptr);
		j["OB_PROP_DEPTH_RM_FILTER_BOOL"] = m_orbbecCtrl.m_depthRMFilter ? json(*m_orbbecCtrl.m_depthRMFilter) : json(nullptr);
		j["OB_PROP_COLOR_AE_MAX_GAIN_INT"] = m_orbbecCtrl.m_colorAEMaxGain ? json(*m_orbbecCtrl.m_colorAEMaxGain) : json(nullptr);
		j["OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT"] = m_orbbecCtrl.m_depthAutoExposurePriority ? json(*m_orbbecCtrl.m_depthAutoExposurePriority) : json(nullptr);
		j["OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL"] = m_orbbecCtrl.m_sdkDisparityToDepth ? json(*m_orbbecCtrl.m_sdkDisparityToDepth) : json(nullptr);
		j["OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL"] = m_orbbecCtrl.m_sdkAccelFrameTransformed ? json(*m_orbbecCtrl.m_sdkAccelFrameTransformed) : json(nullptr);
		j["OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL"] = m_orbbecCtrl.m_sdkGyroFrameTransformed ? json(*m_orbbecCtrl.m_sdkGyroFrameTransformed) : json(nullptr);
		j["OB_PROP_DEVICE_PERFORMANCE_MODE_INT"] = m_orbbecCtrl.m_devicePerformanceMode ? json(*m_orbbecCtrl.m_devicePerformanceMode) : json(nullptr);
		j["OB_PROP_COLOR_DENOISING_LEVEL_INT"] = m_orbbecCtrl.m_colorDenoisingLevel ? json(*m_orbbecCtrl.m_colorDenoisingLevel) : json(nullptr);
		j["OB_STRUCT_MULTI_DEVICE_SYNC_CONFIG"] = m_orbbecCtrl.m_multiDeviceSyncConfig ? json(*m_orbbecCtrl.m_multiDeviceSyncConfig) : json(nullptr);
		j["OB_STRUCT_DEVICE_IP_ADDR_CONFIG"] = m_orbbecCtrl.m_deviceIPAddrConfig ? json(*m_orbbecCtrl.m_deviceIPAddrConfig) : json(nullptr);
		j["OB_STRUCT_DEPTH_HDR_CONFIG"] = m_orbbecCtrl.m_depthHdrConfig ? json(*m_orbbecCtrl.m_depthHdrConfig) : json(nullptr);
		j["OB_STRUCT_COLOR_AE_ROI"] = m_orbbecCtrl.m_colorAEROI ? json(*m_orbbecCtrl.m_colorAEROI) : json(nullptr);
		j["OB_STRUCT_DEPTH_AE_ROI"] = m_orbbecCtrl.m_depthAEROI ? json(*m_orbbecCtrl.m_depthAEROI) : json(nullptr);
		j["OB_STRUCT_DISP_OFFSET_CONFIG"] = m_orbbecCtrl.m_dispOffsetConfig ? json(*m_orbbecCtrl.m_dispOffsetConfig) : json(nullptr);
		j["OB_STRUCT_PRESET_RESOLUTION_CONFIG"] = m_orbbecCtrl.m_presetResolutionConfig ? json(*m_orbbecCtrl.m_presetResolutionConfig) : json(nullptr);
		j["OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2"] = m_orbbecCtrl.m_deviceIPAddrConfigV2 ? json(*m_orbbecCtrl.m_deviceIPAddrConfigV2) : json(nullptr);
		j["obTimestampResetConfig"] = m_orbbecCtrl.m_timestampResetConfig ? json(*m_orbbecCtrl.m_timestampResetConfig) : json(nullptr);
		j["obDepthWorkMode"] = m_orbbecCtrl.m_depthWorkMode ? json(*m_orbbecCtrl.m_depthWorkMode) : json(nullptr);
		j["obColorPreset"] = m_orbbecCtrl.m_colorPreset ? json(*m_orbbecCtrl.m_colorPreset) : json(nullptr);
		j["obPreset"] = m_orbbecCtrl.m_preset ? json(*m_orbbecCtrl.m_preset) : json(nullptr);
		j["obPresetJsonFile"] = m_orbbecCtrl.m_presetJsonFile ? json(*m_orbbecCtrl.m_presetJsonFile) : json(nullptr);
		j["obFrameInterleave"] = m_orbbecCtrl.m_frameInterleave ? json(*m_orbbecCtrl.m_frameInterleave) : json(nullptr);
		j["obGlobalTimestamp"] = m_orbbecCtrl.m_globalTimestamp ? json(*m_orbbecCtrl.m_globalTimestamp) : json(nullptr);
		j["obFirmwareLog"] = m_orbbecCtrl.m_firmwareLog ? json(*m_orbbecCtrl.m_firmwareLog) : json(nullptr);
		return j;
	}

	json _Orbbec::controlSchema(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		json schema = json::array();
		const json values = configValues();
		for (const string key : {"SN", "bNetDevEnum", "tOutMs", "bRGB", "bDepth", "bIMU", "bPCL", "bPCLrgb", "devFPS", "devFPSd", "vSizeRGB", "vSizeD", "dScale"})
		{
			const auto &v = values[key];
			schema.push_back({{"key", key}, {"type", v.is_boolean() ? "bool" : v.is_string() ? "string" : v.is_array() ? "size" : v.is_number_integer() ? "int" : "float"},
				{"category", "Streams and point cloud"}, {"nullable", false}, {"restart", key != "tOutMs" && key != "dScale"}});
		}
		static const json controls = {
			{{"key", "OB_PROP_LDP_BOOL"}, {"type", "bool"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_LDP_BOOL}},
			{{"key", "OB_PROP_LASER_BOOL"}, {"type", "bool"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_LASER_BOOL}},
			{{"key", "OB_PROP_FLOOD_BOOL"}, {"type", "bool"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_FLOOD_BOOL}},
			{{"key", "OB_PROP_TEMPERATURE_COMPENSATION_BOOL"}, {"type", "bool"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_TEMPERATURE_COMPENSATION_BOOL}},
			{{"key", "OB_PROP_DEPTH_MIRROR_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_MIRROR_BOOL}},
			{{"key", "OB_PROP_DEPTH_FLIP_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_FLIP_BOOL}},
			{{"key", "OB_PROP_DEPTH_HOLEFILTER_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_HOLEFILTER_BOOL}},
			{{"key", "OB_PROP_IR_MIRROR_BOOL"}, {"type", "bool"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_MIRROR_BOOL}},
			{{"key", "OB_PROP_IR_FLIP_BOOL"}, {"type", "bool"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_FLIP_BOOL}},
			{{"key", "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL}},
			{{"key", "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT}},
			{{"key", "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT}},
			{{"key", "OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL}},
			{{"key", "OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT}},
			{{"key", "OB_PROP_DEPTH_PRECISION_LEVEL_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_PRECISION_LEVEL_INT}},
			{{"key", "OB_PROP_COLOR_MIRROR_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_MIRROR_BOOL}},
			{{"key", "OB_PROP_COLOR_FLIP_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_FLIP_BOOL}},
			{{"key", "OB_PROP_DISPARITY_TO_DEPTH_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DISPARITY_TO_DEPTH_BOOL}},
			{{"key", "OB_PROP_WATCHDOG_BOOL"}, {"type", "bool"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_WATCHDOG_BOOL}},
			{{"key", "OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL"}, {"type", "bool"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL}},
			{{"key", "OB_PROP_HEARTBEAT_BOOL"}, {"type", "bool"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_HEARTBEAT_BOOL}},
			{{"key", "OB_PROP_LASER_POWER_LEVEL_CONTROL_INT"}, {"type", "int"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_LASER_POWER_LEVEL_CONTROL_INT}},
			{{"key", "OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL"}, {"type", "bool"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL}},
			{{"key", "OB_PROP_TIMER_RESET_DELAY_US_INT"}, {"type", "int"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_PROP_TIMER_RESET_DELAY_US_INT}},
			{{"key", "OB_PROP_IR_RIGHT_MIRROR_BOOL"}, {"type", "bool"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_RIGHT_MIRROR_BOOL}},
			{{"key", "OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT}},
			{{"key", "OB_PROP_IR_RIGHT_FLIP_BOOL"}, {"type", "bool"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_RIGHT_FLIP_BOOL}},
			{{"key", "OB_PROP_COLOR_ROTATE_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_ROTATE_INT}},
			{{"key", "OB_PROP_IR_ROTATE_INT"}, {"type", "int"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_ROTATE_INT}},
			{{"key", "OB_PROP_IR_RIGHT_ROTATE_INT"}, {"type", "int"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_RIGHT_ROTATE_INT}},
			{{"key", "OB_PROP_DEPTH_ROTATE_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_ROTATE_INT}},
			{{"key", "OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL"}, {"type", "bool"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL}},
			{{"key", "OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL"}, {"type", "bool"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL}},
			{{"key", "OB_PROP_LASER_ALWAYS_ON_BOOL"}, {"type", "bool"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_LASER_ALWAYS_ON_BOOL}},
			{{"key", "OB_PROP_LASER_ON_OFF_PATTERN_INT"}, {"type", "int"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_LASER_ON_OFF_PATTERN_INT}},
			{{"key", "OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT"}, {"type", "float"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT}},
			{{"key", "OB_PROP_LASER_CONTROL_INT"}, {"type", "int"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_LASER_CONTROL_INT}},
			{{"key", "OB_PROP_IR_BRIGHTNESS_INT"}, {"type", "int"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_BRIGHTNESS_INT}},
			{{"key", "OB_PROP_COLOR_AE_MAX_EXPOSURE_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_AE_MAX_EXPOSURE_INT}},
			{{"key", "OB_PROP_IR_AE_MAX_EXPOSURE_INT"}, {"type", "int"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_AE_MAX_EXPOSURE_INT}},
			{{"key", "OB_PROP_DISP_SEARCH_RANGE_MODE_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DISP_SEARCH_RANGE_MODE_INT}},
			{{"key", "OB_PROP_DISP_SEARCH_OFFSET_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DISP_SEARCH_OFFSET_INT}},
			{{"key", "OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL"}, {"type", "bool"}, {"category", "Emitter and calibration"}, {"nullable", true}, {"property", OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL}},
			{{"key", "OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT"}, {"type", "int"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT}},
			{{"key", "OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL"}, {"type", "bool"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL}},
			{{"key", "OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT"}, {"type", "int"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT}},
			{{"key", "OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL"}, {"type", "bool"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL}},
			{{"key", "OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL}},
			{{"key", "OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT"}, {"type", "float"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT}},
			{{"key", "OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL}},
			{{"key", "OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT}},
			{{"key", "OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL"}, {"type", "bool"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL}},
			{{"key", "OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL"}, {"type", "bool"}, {"category", "Confidence"}, {"nullable", true}, {"property", OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL}},
			{{"key", "OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT"}, {"type", "int"}, {"category", "Confidence"}, {"nullable", true}, {"property", OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT}},
			{{"key", "OB_PROP_CONFIDENCE_MIRROR_BOOL"}, {"type", "bool"}, {"category", "Confidence"}, {"nullable", true}, {"property", OB_PROP_CONFIDENCE_MIRROR_BOOL}},
			{{"key", "OB_PROP_CONFIDENCE_FLIP_BOOL"}, {"type", "bool"}, {"category", "Confidence"}, {"nullable", true}, {"property", OB_PROP_CONFIDENCE_FLIP_BOOL}},
			{{"key", "OB_PROP_CONFIDENCE_ROTATE_INT"}, {"type", "int"}, {"category", "Confidence"}, {"nullable", true}, {"property", OB_PROP_CONFIDENCE_ROTATE_INT}},
			{{"key", "OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT"}, {"type", "int"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT}},
			{{"key", "OB_PROP_COLOR_RIGHT_ROTATE_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_RIGHT_ROTATE_INT}},
			{{"key", "OB_PROP_COLOR_RIGHT_MIRROR_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_RIGHT_MIRROR_BOOL}},
			{{"key", "OB_PROP_COLOR_RIGHT_FLIP_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_RIGHT_FLIP_BOOL}},
			{{"key", "OB_PROP_DEVICE_AE_REFERENCE_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEVICE_AE_REFERENCE_INT}},
			{{"key", "OB_PROP_DEVICE_AE_STRATEGY_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEVICE_AE_STRATEGY_INT}},
			{{"key", "OB_PROP_COLOR_ROI_BRIGHTNESS_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_ROI_BRIGHTNESS_INT}},
			{{"key", "OB_PROP_COLOR_LEFT_ROTATE_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_LEFT_ROTATE_INT}},
			{{"key", "OB_PROP_COLOR_LEFT_MIRROR_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_LEFT_MIRROR_BOOL}},
			{{"key", "OB_PROP_COLOR_LEFT_FLIP_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_LEFT_FLIP_BOOL}},
			{{"key", "OB_PROP_COLOR_PRESET_PRIORITY_INT"}, {"type", "int"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_COLOR_PRESET_PRIORITY_INT}},
			{{"key", "OB_PROP_DEVICE_NETWORK_LLA_BOOL"}, {"type", "bool"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_DEVICE_NETWORK_LLA_BOOL}},
			{{"key", "OB_PROP_COLOR_ANTI_FLICKER_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_ANTI_FLICKER_BOOL}},
			{{"key", "OB_PROP_DEVICE_IP_MODE_INT"}, {"type", "int"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_DEVICE_IP_MODE_INT}},
			{{"key", "OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT"}, {"type", "int"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT}},
			{{"key", "OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT"}, {"type", "int"}, {"category", "Timing and synchronization"}, {"nullable", true}, {"property", OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT}},
			{{"key", "OB_PROP_FPS_BOOST_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_FPS_BOOST_BOOL}},
			{{"key", "OB_PROP_MJPEG_QUALITY_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_MJPEG_QUALITY_INT}},
			{{"key", "OB_PROP_COLOR_AUTO_EXPOSURE_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_AUTO_EXPOSURE_BOOL}},
			{{"key", "OB_PROP_COLOR_EXPOSURE_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_EXPOSURE_INT}},
			{{"key", "OB_PROP_COLOR_GAIN_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_GAIN_INT}},
			{{"key", "OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL"}, {"type", "bool"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL}},
			{{"key", "OB_PROP_COLOR_WHITE_BALANCE_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_WHITE_BALANCE_INT}},
			{{"key", "OB_PROP_COLOR_BRIGHTNESS_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_BRIGHTNESS_INT}},
			{{"key", "OB_PROP_COLOR_SHARPNESS_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_SHARPNESS_INT}},
			{{"key", "OB_PROP_COLOR_SATURATION_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_SATURATION_INT}},
			{{"key", "OB_PROP_COLOR_CONTRAST_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_CONTRAST_INT}},
			{{"key", "OB_PROP_COLOR_GAMMA_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_GAMMA_INT}},
			{{"key", "OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT}},
			{{"key", "OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT}},
			{{"key", "OB_PROP_COLOR_HUE_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_HUE_INT}},
			{{"key", "OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT}},
			{{"key", "OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL}},
			{{"key", "OB_PROP_DEPTH_EXPOSURE_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_EXPOSURE_INT}},
			{{"key", "OB_PROP_DEPTH_GAIN_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_GAIN_INT}},
			{{"key", "OB_PROP_IR_AUTO_EXPOSURE_BOOL"}, {"type", "bool"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_AUTO_EXPOSURE_BOOL}},
			{{"key", "OB_PROP_IR_EXPOSURE_INT"}, {"type", "int"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_EXPOSURE_INT}},
			{{"key", "OB_PROP_IR_GAIN_INT"}, {"type", "int"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_GAIN_INT}},
			{{"key", "OB_PROP_IR_CHANNEL_DATA_SOURCE_INT"}, {"type", "int"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_IR_CHANNEL_DATA_SOURCE_INT}},
			{{"key", "OB_PROP_DEPTH_RM_FILTER_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_RM_FILTER_BOOL}},
			{{"key", "OB_PROP_COLOR_AE_MAX_GAIN_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_AE_MAX_GAIN_INT}},
			{{"key", "OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT"}, {"type", "int"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT}},
			{{"key", "OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL"}, {"type", "bool"}, {"category", "Depth and filtering"}, {"nullable", true}, {"property", OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL}},
			{{"key", "OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL"}, {"type", "bool"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL}},
			{{"key", "OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL"}, {"type", "bool"}, {"category", "Infrared and IMU"}, {"nullable", true}, {"property", OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL}},
			{{"key", "OB_PROP_DEVICE_PERFORMANCE_MODE_INT"}, {"type", "int"}, {"category", "Device and network"}, {"nullable", true}, {"property", OB_PROP_DEVICE_PERFORMANCE_MODE_INT}},
			{{"key", "OB_PROP_COLOR_DENOISING_LEVEL_INT"}, {"type", "int"}, {"category", "Color"}, {"nullable", true}, {"property", OB_PROP_COLOR_DENOISING_LEVEL_INT}},
			{{"key", "OB_STRUCT_MULTI_DEVICE_SYNC_CONFIG"}, {"type", "object"}, {"category", "Timing and synchronization"}, {"nullable", true}},
			{{"key", "OB_STRUCT_DEVICE_IP_ADDR_CONFIG"}, {"type", "object"}, {"category", "Device and network"}, {"nullable", true}},
			{{"key", "OB_STRUCT_DEPTH_HDR_CONFIG"}, {"type", "object"}, {"category", "Depth and filtering"}, {"nullable", true}},
			{{"key", "OB_STRUCT_COLOR_AE_ROI"}, {"type", "object"}, {"category", "Color"}, {"nullable", true}},
			{{"key", "OB_STRUCT_DEPTH_AE_ROI"}, {"type", "object"}, {"category", "Depth and filtering"}, {"nullable", true}},
			{{"key", "OB_STRUCT_DISP_OFFSET_CONFIG"}, {"type", "object"}, {"category", "Depth and filtering"}, {"nullable", true}},
			{{"key", "OB_STRUCT_PRESET_RESOLUTION_CONFIG"}, {"type", "object"}, {"category", "Device and network"}, {"nullable", true}},
			{{"key", "OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2"}, {"type", "object"}, {"category", "Device and network"}, {"nullable", true}},
			{{"key", "obTimestampResetConfig"}, {"type", "object"}, {"category", "Timing and synchronization"}, {"nullable", true}},
			{{"key", "obDepthWorkMode"}, {"type", "string"}, {"category", "Presets"}, {"nullable", true}},
			{{"key", "obColorPreset"}, {"type", "string"}, {"category", "Color"}, {"nullable", true}},
			{{"key", "obPreset"}, {"type", "string"}, {"category", "Presets"}, {"nullable", true}},
			{{"key", "obPresetJsonFile"}, {"type", "string"}, {"category", "Presets"}, {"nullable", true}},
			{{"key", "obFrameInterleave"}, {"type", "string"}, {"category", "Timing and synchronization"}, {"nullable", true}},
			{{"key", "obGlobalTimestamp"}, {"type", "bool"}, {"category", "Timing and synchronization"}, {"nullable", true}},
			{{"key", "obFirmwareLog"}, {"type", "bool"}, {"category", "Device and network"}, {"nullable", true}},
		};
		for (json field : controls)
		{
			// Device readback is informational; it never creates an override.
			if (m_spDev && field.contains("property"))
			{
				const auto id = field["property"].get<OBPropertyID>();
				const string type = field["type"].get<string>();
				try
				{
					field["supported"] = m_spDev->isPropertySupported(id, OB_PERMISSION_WRITE);
					if (m_spDev->isPropertySupported(id, OB_PERMISSION_READ))
					{
						if (type == "bool")
							field["current"] = m_spDev->getBoolProperty(id);
						else if (type == "int")
						{
							const auto range = m_spDev->getIntPropertyRange(id);
							field["min"] = range.min; field["max"] = range.max;
							field["step"] = range.step; field["current"] = range.cur;
						}
						else if (type == "float")
						{
							const auto range = m_spDev->getFloatPropertyRange(id);
							field["min"] = range.min; field["max"] = range.max;
							field["step"] = range.step; field["current"] = range.cur;
						}
					}
				}
				catch (const ob::Error &e) { field["note"] = e.what(); }
			}
			field.erase("property");
			schema.push_back(std::move(field));
		}
		static const json examples = json::parse(R"json({
			"OB_STRUCT_MULTI_DEVICE_SYNC_CONFIG": {"syncMode": 1, "depthDelayUs": 0, "colorDelayUs": 0, "trigger2ImageDelayUs": 0, "triggerOutEnable": false, "triggerOutDelayUs": 0, "framesPerTrigger": 1},
			"OB_STRUCT_DEVICE_IP_ADDR_CONFIG": {"dhcp": true, "address": [192, 168, 1, 10], "mask": [255, 255, 255, 0], "gateway": [192, 168, 1, 1]},
			"OB_STRUCT_DEPTH_HDR_CONFIG": {"enable": false, "sequence_name": 0, "exposure_1": 1000, "gain_1": 16, "exposure_2": 100, "gain_2": 16},
			"OB_STRUCT_COLOR_AE_ROI": {"x0_left": 0, "y0_top": 0, "x1_right": 639, "y1_bottom": 479},
			"OB_STRUCT_DEPTH_AE_ROI": {"x0_left": 0, "y0_top": 0, "x1_right": 639, "y1_bottom": 479},
			"OB_STRUCT_DISP_OFFSET_CONFIG": {"enable": false, "offset0": 0, "offset1": 0},
			"OB_STRUCT_PRESET_RESOLUTION_CONFIG": {"width": 640, "height": 480, "irDecimationFactor": 1, "depthDecimationFactor": 1},
			"OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2": {"flags": 0, "address": [192, 168, 1, 10], "mask": [255, 255, 255, 0], "gateway": [192, 168, 1, 1]},
			"obTimestampResetConfig": {"enable": false, "timestamp_reset_delay_us": 0, "timestamp_reset_signal_output_enable": false}
		})json");
		for (auto &field : schema) if (examples.contains(field["key"])) field["example"] = examples[field["key"]];
		return schema;
	}

	void _Orbbec::setStreamConfig(const json &j)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_SN = j.at("SN").get<string>();
		m_bNetDevEnum = j.at("bNetDevEnum").get<bool>();
		m_tOutMs = j.at("tOutMs").get<uint32_t>();
		m_bRGB = j.at("bRGB").get<bool>(); m_bDepth = j.at("bDepth").get<bool>();
		m_bIMU = j.at("bIMU").get<bool>(); m_bPCL = j.at("bPCL").get<bool>(); m_bPCLrgb = j.at("bPCLrgb").get<bool>();
		m_devFPS = j.at("devFPS").get<int>(); m_devFPSd = j.at("devFPSd").get<int>();
		m_vSizeRGB = Vector2i(j.at("vSizeRGB")[0].get<int>(), j.at("vSizeRGB")[1].get<int>());
		m_vSizeD = Vector2i(j.at("vSizeD")[0].get<int>(), j.at("vSizeD")[1].get<int>());
		m_dScale = j.at("dScale").get<float>();
	}

	bool _Orbbec::applyConfig(const json &values, bool live, json &errors)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		errors = json::object();
		if (!values.is_object()) { errors["config"] = "Expected an object"; return false; }
		const json before = configValues();
		json next = before;
		OrbbecCtrl candidate = m_orbbecCtrl;
		bool restart = false;
		// Validate the entire patch before changing any state or touching hardware.
		for (auto it = values.begin(); it != values.end(); ++it)
		{
			const auto &key = it.key(); const auto &v = it.value();
			try
			{
				if (!before.contains(key)) throw std::invalid_argument("Unknown parameter");
				if (key == "OB_PROP_LDP_BOOL")
				{
					if (v.is_null()) candidate.m_ldp.reset();
					else candidate.m_ldp = readControl<bool>(v);
				}
				else if (key == "OB_PROP_LASER_BOOL")
				{
					if (v.is_null()) candidate.m_laser.reset();
					else candidate.m_laser = readControl<bool>(v);
				}
				else if (key == "OB_PROP_FLOOD_BOOL")
				{
					if (v.is_null()) candidate.m_flood.reset();
					else candidate.m_flood = readControl<bool>(v);
				}
				else if (key == "OB_PROP_TEMPERATURE_COMPENSATION_BOOL")
				{
					if (v.is_null()) candidate.m_temperatureCompensation.reset();
					else candidate.m_temperatureCompensation = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEPTH_MIRROR_BOOL")
				{
					if (v.is_null()) candidate.m_depthMirror.reset();
					else candidate.m_depthMirror = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEPTH_FLIP_BOOL")
				{
					if (v.is_null()) candidate.m_depthFlip.reset();
					else candidate.m_depthFlip = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEPTH_HOLEFILTER_BOOL")
				{
					if (v.is_null()) candidate.m_depthHolefilter.reset();
					else candidate.m_depthHolefilter = readControl<bool>(v);
				}
				else if (key == "OB_PROP_IR_MIRROR_BOOL")
				{
					if (v.is_null()) candidate.m_irMirror.reset();
					else candidate.m_irMirror = readControl<bool>(v);
				}
				else if (key == "OB_PROP_IR_FLIP_BOOL")
				{
					if (v.is_null()) candidate.m_irFlip.reset();
					else candidate.m_irFlip = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL")
				{
					if (v.is_null()) candidate.m_depthNoiseRemovalFilter.reset();
					else candidate.m_depthNoiseRemovalFilter = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT")
				{
					if (v.is_null()) candidate.m_depthNoiseRemovalFilterMaxDiff.reset();
					else candidate.m_depthNoiseRemovalFilterMaxDiff = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT")
				{
					if (v.is_null()) candidate.m_depthNoiseRemovalFilterMaxSpeckleSize.reset();
					else candidate.m_depthNoiseRemovalFilterMaxSpeckleSize = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL")
				{
					if (v.is_null()) candidate.m_depthAlignHardware.reset();
					else candidate.m_depthAlignHardware = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT")
				{
					if (v.is_null()) candidate.m_depthAlignHardwareMode.reset();
					else candidate.m_depthAlignHardwareMode = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_PRECISION_LEVEL_INT")
				{
					if (v.is_null()) candidate.m_depthPrecisionLevel.reset();
					else candidate.m_depthPrecisionLevel = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_MIRROR_BOOL")
				{
					if (v.is_null()) candidate.m_colorMirror.reset();
					else candidate.m_colorMirror = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_FLIP_BOOL")
				{
					if (v.is_null()) candidate.m_colorFlip.reset();
					else candidate.m_colorFlip = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DISPARITY_TO_DEPTH_BOOL")
				{
					if (v.is_null()) candidate.m_disparityToDepth.reset();
					else candidate.m_disparityToDepth = readControl<bool>(v);
				}
				else if (key == "OB_PROP_WATCHDOG_BOOL")
				{
					if (v.is_null()) candidate.m_watchdog.reset();
					else candidate.m_watchdog = readControl<bool>(v);
				}
				else if (key == "OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL")
				{
					if (v.is_null()) candidate.m_externalSignalReset.reset();
					else candidate.m_externalSignalReset = readControl<bool>(v);
				}
				else if (key == "OB_PROP_HEARTBEAT_BOOL")
				{
					if (v.is_null()) candidate.m_heartbeat.reset();
					else candidate.m_heartbeat = readControl<bool>(v);
				}
				else if (key == "OB_PROP_LASER_POWER_LEVEL_CONTROL_INT")
				{
					if (v.is_null()) candidate.m_laserPowerLevelControl.reset();
					else candidate.m_laserPowerLevelControl = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL")
				{
					if (v.is_null()) candidate.m_timerResetTriggerOutEnable.reset();
					else candidate.m_timerResetTriggerOutEnable = readControl<bool>(v);
				}
				else if (key == "OB_PROP_TIMER_RESET_DELAY_US_INT")
				{
					if (v.is_null()) candidate.m_timerResetDelayUs.reset();
					else candidate.m_timerResetDelayUs = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_RIGHT_MIRROR_BOOL")
				{
					if (v.is_null()) candidate.m_irRightMirror.reset();
					else candidate.m_irRightMirror = readControl<bool>(v);
				}
				else if (key == "OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT")
				{
					if (v.is_null()) candidate.m_captureImageFrameNumber.reset();
					else candidate.m_captureImageFrameNumber = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_RIGHT_FLIP_BOOL")
				{
					if (v.is_null()) candidate.m_irRightFlip.reset();
					else candidate.m_irRightFlip = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_ROTATE_INT")
				{
					if (v.is_null()) candidate.m_colorRotate.reset();
					else candidate.m_colorRotate = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_ROTATE_INT")
				{
					if (v.is_null()) candidate.m_irRotate.reset();
					else candidate.m_irRotate = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_RIGHT_ROTATE_INT")
				{
					if (v.is_null()) candidate.m_irRightRotate.reset();
					else candidate.m_irRightRotate = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_ROTATE_INT")
				{
					if (v.is_null()) candidate.m_depthRotate.reset();
					else candidate.m_depthRotate = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL")
				{
					if (v.is_null()) candidate.m_syncSignalTriggerOut.reset();
					else candidate.m_syncSignalTriggerOut = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL")
				{
					if (v.is_null()) candidate.m_deviceUSB2RepeatIdentify.reset();
					else candidate.m_deviceUSB2RepeatIdentify = readControl<bool>(v);
				}
				else if (key == "OB_PROP_LASER_ALWAYS_ON_BOOL")
				{
					if (v.is_null()) candidate.m_laserAlwaysOn.reset();
					else candidate.m_laserAlwaysOn = readControl<bool>(v);
				}
				else if (key == "OB_PROP_LASER_ON_OFF_PATTERN_INT")
				{
					if (v.is_null()) candidate.m_laserOnOffPattern.reset();
					else candidate.m_laserOnOffPattern = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT")
				{
					if (v.is_null()) candidate.m_depthUnitFlexibleAdjustment.reset();
					else candidate.m_depthUnitFlexibleAdjustment = readControl<float>(v);
				}
				else if (key == "OB_PROP_LASER_CONTROL_INT")
				{
					if (v.is_null()) candidate.m_laserControl.reset();
					else candidate.m_laserControl = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_BRIGHTNESS_INT")
				{
					if (v.is_null()) candidate.m_irBrightness.reset();
					else candidate.m_irBrightness = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_AE_MAX_EXPOSURE_INT")
				{
					if (v.is_null()) candidate.m_colorAEMaxExposure.reset();
					else candidate.m_colorAEMaxExposure = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_AE_MAX_EXPOSURE_INT")
				{
					if (v.is_null()) candidate.m_irAEMaxExposure.reset();
					else candidate.m_irAEMaxExposure = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DISP_SEARCH_RANGE_MODE_INT")
				{
					if (v.is_null()) candidate.m_dispSearchRangeMode.reset();
					else candidate.m_dispSearchRangeMode = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DISP_SEARCH_OFFSET_INT")
				{
					if (v.is_null()) candidate.m_dispSearchOffset.reset();
					else candidate.m_dispSearchOffset = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL")
				{
					if (v.is_null()) candidate.m_cpuTemperatureCalibration.reset();
					else candidate.m_cpuTemperatureCalibration = readControl<bool>(v);
				}
				else if (key == "OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT")
				{
					if (v.is_null()) candidate.m_frameInterleaveConfigIndex.reset();
					else candidate.m_frameInterleaveConfigIndex = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL")
				{
					if (v.is_null()) candidate.m_frameInterleaveEnable.reset();
					else candidate.m_frameInterleaveEnable = readControl<bool>(v);
				}
				else if (key == "OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT")
				{
					if (v.is_null()) candidate.m_frameInterleaveLaserPatternSyncDelay.reset();
					else candidate.m_frameInterleaveLaserPatternSyncDelay = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL")
				{
					if (v.is_null()) candidate.m_onChipCalibrationEnable.reset();
					else candidate.m_onChipCalibrationEnable = readControl<bool>(v);
				}
				else if (key == "OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL")
				{
					if (v.is_null()) candidate.m_hwNoiseRemoveFilterEnable.reset();
					else candidate.m_hwNoiseRemoveFilterEnable = readControl<bool>(v);
				}
				else if (key == "OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT")
				{
					if (v.is_null()) candidate.m_hwNoiseRemoveFilterThreshold.reset();
					else candidate.m_hwNoiseRemoveFilterThreshold = readControl<float>(v);
				}
				else if (key == "OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL")
				{
					if (v.is_null()) candidate.m_deviceAutoCaptureEnable.reset();
					else candidate.m_deviceAutoCaptureEnable = readControl<bool>(v);
				}
				else if (key == "OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT")
				{
					if (v.is_null()) candidate.m_deviceAutoCaptureIntervalTime.reset();
					else candidate.m_deviceAutoCaptureIntervalTime = readControl<int32_t>(v);
				}
				else if (key == "OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL")
				{
					if (v.is_null()) candidate.m_devicePTPClockSyncEnable.reset();
					else candidate.m_devicePTPClockSyncEnable = readControl<bool>(v);
				}
				else if (key == "OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL")
				{
					if (v.is_null()) candidate.m_confidenceStreamFilter.reset();
					else candidate.m_confidenceStreamFilter = readControl<bool>(v);
				}
				else if (key == "OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT")
				{
					if (v.is_null()) candidate.m_confidenceStreamFilterThreshold.reset();
					else candidate.m_confidenceStreamFilterThreshold = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_CONFIDENCE_MIRROR_BOOL")
				{
					if (v.is_null()) candidate.m_confidenceMirror.reset();
					else candidate.m_confidenceMirror = readControl<bool>(v);
				}
				else if (key == "OB_PROP_CONFIDENCE_FLIP_BOOL")
				{
					if (v.is_null()) candidate.m_confidenceFlip.reset();
					else candidate.m_confidenceFlip = readControl<bool>(v);
				}
				else if (key == "OB_PROP_CONFIDENCE_ROTATE_INT")
				{
					if (v.is_null()) candidate.m_confidenceRotate.reset();
					else candidate.m_confidenceRotate = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT")
				{
					if (v.is_null()) candidate.m_intraCameraSyncReference.reset();
					else candidate.m_intraCameraSyncReference = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_RIGHT_ROTATE_INT")
				{
					if (v.is_null()) candidate.m_colorRightRotate.reset();
					else candidate.m_colorRightRotate = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_RIGHT_MIRROR_BOOL")
				{
					if (v.is_null()) candidate.m_colorRightMirror.reset();
					else candidate.m_colorRightMirror = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_RIGHT_FLIP_BOOL")
				{
					if (v.is_null()) candidate.m_colorRightFlip.reset();
					else candidate.m_colorRightFlip = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEVICE_AE_REFERENCE_INT")
				{
					if (v.is_null()) candidate.m_deviceAEReference.reset();
					else candidate.m_deviceAEReference = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEVICE_AE_STRATEGY_INT")
				{
					if (v.is_null()) candidate.m_deviceAEStrategy.reset();
					else candidate.m_deviceAEStrategy = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_ROI_BRIGHTNESS_INT")
				{
					if (v.is_null()) candidate.m_colorROIBrightness.reset();
					else candidate.m_colorROIBrightness = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_LEFT_ROTATE_INT")
				{
					if (v.is_null()) candidate.m_colorLeftRotate.reset();
					else candidate.m_colorLeftRotate = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_LEFT_MIRROR_BOOL")
				{
					if (v.is_null()) candidate.m_colorLeftMirror.reset();
					else candidate.m_colorLeftMirror = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_LEFT_FLIP_BOOL")
				{
					if (v.is_null()) candidate.m_colorLeftFlip.reset();
					else candidate.m_colorLeftFlip = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_PRESET_PRIORITY_INT")
				{
					if (v.is_null()) candidate.m_colorPresetPriority.reset();
					else candidate.m_colorPresetPriority = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEVICE_NETWORK_LLA_BOOL")
				{
					if (v.is_null()) candidate.m_deviceNetworkLLA.reset();
					else candidate.m_deviceNetworkLLA = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_ANTI_FLICKER_BOOL")
				{
					if (v.is_null()) candidate.m_colorAntiFlicker.reset();
					else candidate.m_colorAntiFlicker = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEVICE_IP_MODE_INT")
				{
					if (v.is_null()) candidate.m_deviceIPMode.reset();
					else candidate.m_deviceIPMode = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT")
				{
					if (v.is_null()) candidate.m_dhcpAssignIPTimeout.reset();
					else candidate.m_dhcpAssignIPTimeout = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT")
				{
					if (v.is_null()) candidate.m_usbSyncVoltageLevel.reset();
					else candidate.m_usbSyncVoltageLevel = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_FPS_BOOST_BOOL")
				{
					if (v.is_null()) candidate.m_fpsBoost.reset();
					else candidate.m_fpsBoost = readControl<bool>(v);
				}
				else if (key == "OB_PROP_MJPEG_QUALITY_INT")
				{
					if (v.is_null()) candidate.m_mjpegQuality.reset();
					else candidate.m_mjpegQuality = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_AUTO_EXPOSURE_BOOL")
				{
					if (v.is_null()) candidate.m_colorAutoExposure.reset();
					else candidate.m_colorAutoExposure = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_EXPOSURE_INT")
				{
					if (v.is_null()) candidate.m_colorExposure.reset();
					else candidate.m_colorExposure = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_GAIN_INT")
				{
					if (v.is_null()) candidate.m_colorGain.reset();
					else candidate.m_colorGain = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL")
				{
					if (v.is_null()) candidate.m_colorAutoWhiteBalance.reset();
					else candidate.m_colorAutoWhiteBalance = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_WHITE_BALANCE_INT")
				{
					if (v.is_null()) candidate.m_colorWhiteBalance.reset();
					else candidate.m_colorWhiteBalance = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_BRIGHTNESS_INT")
				{
					if (v.is_null()) candidate.m_colorBrightness.reset();
					else candidate.m_colorBrightness = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_SHARPNESS_INT")
				{
					if (v.is_null()) candidate.m_colorSharpness.reset();
					else candidate.m_colorSharpness = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_SATURATION_INT")
				{
					if (v.is_null()) candidate.m_colorSaturation.reset();
					else candidate.m_colorSaturation = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_CONTRAST_INT")
				{
					if (v.is_null()) candidate.m_colorContrast.reset();
					else candidate.m_colorContrast = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_GAMMA_INT")
				{
					if (v.is_null()) candidate.m_colorGamma.reset();
					else candidate.m_colorGamma = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT")
				{
					if (v.is_null()) candidate.m_colorAutoExposurePriority.reset();
					else candidate.m_colorAutoExposurePriority = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT")
				{
					if (v.is_null()) candidate.m_colorBacklightCompensation.reset();
					else candidate.m_colorBacklightCompensation = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_HUE_INT")
				{
					if (v.is_null()) candidate.m_colorHue.reset();
					else candidate.m_colorHue = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT")
				{
					if (v.is_null()) candidate.m_colorPowerLineFrequency.reset();
					else candidate.m_colorPowerLineFrequency = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL")
				{
					if (v.is_null()) candidate.m_depthAutoExposure.reset();
					else candidate.m_depthAutoExposure = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEPTH_EXPOSURE_INT")
				{
					if (v.is_null()) candidate.m_depthExposure.reset();
					else candidate.m_depthExposure = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_GAIN_INT")
				{
					if (v.is_null()) candidate.m_depthGain.reset();
					else candidate.m_depthGain = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_AUTO_EXPOSURE_BOOL")
				{
					if (v.is_null()) candidate.m_irAutoExposure.reset();
					else candidate.m_irAutoExposure = readControl<bool>(v);
				}
				else if (key == "OB_PROP_IR_EXPOSURE_INT")
				{
					if (v.is_null()) candidate.m_irExposure.reset();
					else candidate.m_irExposure = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_GAIN_INT")
				{
					if (v.is_null()) candidate.m_irGain.reset();
					else candidate.m_irGain = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_IR_CHANNEL_DATA_SOURCE_INT")
				{
					if (v.is_null()) candidate.m_irChannelDataSource.reset();
					else candidate.m_irChannelDataSource = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_RM_FILTER_BOOL")
				{
					if (v.is_null()) candidate.m_depthRMFilter.reset();
					else candidate.m_depthRMFilter = readControl<bool>(v);
				}
				else if (key == "OB_PROP_COLOR_AE_MAX_GAIN_INT")
				{
					if (v.is_null()) candidate.m_colorAEMaxGain.reset();
					else candidate.m_colorAEMaxGain = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT")
				{
					if (v.is_null()) candidate.m_depthAutoExposurePriority.reset();
					else candidate.m_depthAutoExposurePriority = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL")
				{
					if (v.is_null()) candidate.m_sdkDisparityToDepth.reset();
					else candidate.m_sdkDisparityToDepth = readControl<bool>(v);
				}
				else if (key == "OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL")
				{
					if (v.is_null()) candidate.m_sdkAccelFrameTransformed.reset();
					else candidate.m_sdkAccelFrameTransformed = readControl<bool>(v);
				}
				else if (key == "OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL")
				{
					if (v.is_null()) candidate.m_sdkGyroFrameTransformed.reset();
					else candidate.m_sdkGyroFrameTransformed = readControl<bool>(v);
				}
				else if (key == "OB_PROP_DEVICE_PERFORMANCE_MODE_INT")
				{
					if (v.is_null()) candidate.m_devicePerformanceMode.reset();
					else candidate.m_devicePerformanceMode = readControl<int32_t>(v);
				}
				else if (key == "OB_PROP_COLOR_DENOISING_LEVEL_INT")
				{
					if (v.is_null()) candidate.m_colorDenoisingLevel.reset();
					else candidate.m_colorDenoisingLevel = readControl<int32_t>(v);
				}
				else if (key == "OB_STRUCT_MULTI_DEVICE_SYNC_CONFIG")
				{
					if (v.is_null()) candidate.m_multiDeviceSyncConfig.reset();
					else candidate.m_multiDeviceSyncConfig = readControl<OBMultiDeviceSyncConfig>(v);
				}
				else if (key == "OB_STRUCT_DEVICE_IP_ADDR_CONFIG")
				{
					if (v.is_null()) candidate.m_deviceIPAddrConfig.reset();
					else candidate.m_deviceIPAddrConfig = readControl<OBNetIpConfig>(v);
				}
				else if (key == "OB_STRUCT_DEPTH_HDR_CONFIG")
				{
					if (v.is_null()) candidate.m_depthHdrConfig.reset();
					else candidate.m_depthHdrConfig = readControl<OBHdrConfig>(v);
				}
				else if (key == "OB_STRUCT_COLOR_AE_ROI")
				{
					if (v.is_null()) candidate.m_colorAEROI.reset();
					else candidate.m_colorAEROI = readControl<OBRegionOfInterest>(v);
				}
				else if (key == "OB_STRUCT_DEPTH_AE_ROI")
				{
					if (v.is_null()) candidate.m_depthAEROI.reset();
					else candidate.m_depthAEROI = readControl<OBRegionOfInterest>(v);
				}
				else if (key == "OB_STRUCT_DISP_OFFSET_CONFIG")
				{
					if (v.is_null()) candidate.m_dispOffsetConfig.reset();
					else candidate.m_dispOffsetConfig = readControl<OBDispOffsetConfig>(v);
				}
				else if (key == "OB_STRUCT_PRESET_RESOLUTION_CONFIG")
				{
					if (v.is_null()) candidate.m_presetResolutionConfig.reset();
					else candidate.m_presetResolutionConfig = readControl<OBPresetResolutionConfig>(v);
				}
				else if (key == "OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2")
				{
					if (v.is_null()) candidate.m_deviceIPAddrConfigV2.reset();
					else candidate.m_deviceIPAddrConfigV2 = readControl<OBNetIpConfigV2>(v);
				}
				else if (key == "obTimestampResetConfig")
				{
					if (v.is_null()) candidate.m_timestampResetConfig.reset();
					else candidate.m_timestampResetConfig = readControl<OBDeviceTimestampResetConfig>(v);
				}
				else if (key == "obDepthWorkMode")
				{
					if (v.is_null()) candidate.m_depthWorkMode.reset();
					else candidate.m_depthWorkMode = readControl<string>(v);
				}
				else if (key == "obColorPreset")
				{
					if (v.is_null()) candidate.m_colorPreset.reset();
					else candidate.m_colorPreset = readControl<string>(v);
				}
				else if (key == "obPreset")
				{
					if (v.is_null()) candidate.m_preset.reset();
					else candidate.m_preset = readControl<string>(v);
				}
				else if (key == "obPresetJsonFile")
				{
					if (v.is_null()) candidate.m_presetJsonFile.reset();
					else candidate.m_presetJsonFile = readControl<string>(v);
				}
				else if (key == "obFrameInterleave")
				{
					if (v.is_null()) candidate.m_frameInterleave.reset();
					else candidate.m_frameInterleave = readControl<string>(v);
				}
				else if (key == "obGlobalTimestamp")
				{
					if (v.is_null()) candidate.m_globalTimestamp.reset();
					else candidate.m_globalTimestamp = readControl<bool>(v);
				}
				else if (key == "obFirmwareLog")
				{
					if (v.is_null()) candidate.m_firmwareLog.reset();
					else candidate.m_firmwareLog = readControl<bool>(v);
				}
				else
				{
					if (before[key].is_boolean()) readControl<bool>(v);
					else if (key == "SN") readControl<string>(v);
					else if (key == "vSizeRGB" || key == "vSizeD")
					{
						if (!v.is_array() || v.size() != 2) throw std::invalid_argument("Expected [width, height]");
						for (const auto &n : v) if (readControl<int>(n) < 1 || n.get<int>() > 16384) throw std::invalid_argument("Invalid image size");
					}
					else if (key == "dScale") { if (readControl<float>(v) <= 0) throw std::invalid_argument("Scale must be positive"); }
					else if (readControl<int>(v) < 1 || v.get<int>() > (key == "tOutMs" ? 10000 : 1000)) throw std::invalid_argument("Value out of range");
					restart |= v != before[key] && key != "tOutMs" && key != "dScale";
				}
				next[key] = v;
			}
			catch (const std::exception &e) { errors[key] = e.what(); }
		}
		if (!errors.empty()) return false;
		if (next["bPCLrgb"].get<bool>() && (!next["bRGB"].get<bool>() || !next["bDepth"].get<bool>()))
		{ errors["bPCLrgb"] = "RGB point cloud requires both RGB and depth streams"; return false; }
		if (next["bPCL"].get<bool>() && !next["bDepth"].get<bool>())
		{ errors["bPCL"] = "Point cloud requires depth"; return false; }
		if (!live) { setStreamConfig(next); m_orbbecCtrl = candidate; return true; }
		if (!m_bOpened) { errors["device"] = "Camera is not open; configuration was not changed"; return false; }
		if (restart)
		{
			close();
			setStreamConfig(next);
			try { if (!open()) throw std::runtime_error("Cannot reopen camera"); }
			catch (const std::exception &e)
			{
				errors["streams"] = e.what(); close(); setStreamConfig(before);
				try { open(); } catch (const std::exception &restore) { LOG_E(restore.what()); close(); }
				return false;
			}
		}
		else setStreamConfig(next);
		// Only write the submitted properties. Reapplying presets on every edit
		// would reset unrelated live camera settings.
		if (values.contains("obDepthWorkMode"))
		{
			const auto previous = m_orbbecCtrl.m_depthWorkMode;
			if (!candidate.m_depthWorkMode)
				m_orbbecCtrl.m_depthWorkMode.reset();
			else if (!switchDepthWorkMode(*candidate.m_depthWorkMode))
			{
				m_orbbecCtrl.m_depthWorkMode = previous;
				errors["obDepthWorkMode"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("obColorPreset"))
		{
			const auto previous = m_orbbecCtrl.m_colorPreset;
			if (!candidate.m_colorPreset)
				m_orbbecCtrl.m_colorPreset.reset();
			else if (!switchColorPreset(*candidate.m_colorPreset))
			{
				m_orbbecCtrl.m_colorPreset = previous;
				errors["obColorPreset"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("obPreset"))
		{
			const auto previous = m_orbbecCtrl.m_preset;
			if (!candidate.m_preset)
				m_orbbecCtrl.m_preset.reset();
			else if (!loadPreset(*candidate.m_preset))
			{
				m_orbbecCtrl.m_preset = previous;
				errors["obPreset"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("obPresetJsonFile"))
		{
			const auto previous = m_orbbecCtrl.m_presetJsonFile;
			if (!candidate.m_presetJsonFile)
				m_orbbecCtrl.m_presetJsonFile.reset();
			else if (!loadPresetFromJsonFile(*candidate.m_presetJsonFile))
			{
				m_orbbecCtrl.m_presetJsonFile = previous;
				errors["obPresetJsonFile"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("obFrameInterleave"))
		{
			const auto previous = m_orbbecCtrl.m_frameInterleave;
			if (!candidate.m_frameInterleave)
				m_orbbecCtrl.m_frameInterleave.reset();
			else if (!loadFrameInterleave(*candidate.m_frameInterleave))
			{
				m_orbbecCtrl.m_frameInterleave = previous;
				errors["obFrameInterleave"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_LDP_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_ldp;
			if (!candidate.m_ldp)
				m_orbbecCtrl.m_ldp.reset();
			else if (!setLDP(*candidate.m_ldp))
			{
				m_orbbecCtrl.m_ldp = previous;
				errors["OB_PROP_LDP_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_LASER_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_laser;
			if (!candidate.m_laser)
				m_orbbecCtrl.m_laser.reset();
			else if (!setLaser(*candidate.m_laser))
			{
				m_orbbecCtrl.m_laser = previous;
				errors["OB_PROP_LASER_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_FLOOD_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_flood;
			if (!candidate.m_flood)
				m_orbbecCtrl.m_flood.reset();
			else if (!setFlood(*candidate.m_flood))
			{
				m_orbbecCtrl.m_flood = previous;
				errors["OB_PROP_FLOOD_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_TEMPERATURE_COMPENSATION_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_temperatureCompensation;
			if (!candidate.m_temperatureCompensation)
				m_orbbecCtrl.m_temperatureCompensation.reset();
			else if (!setTemperatureCompensation(*candidate.m_temperatureCompensation))
			{
				m_orbbecCtrl.m_temperatureCompensation = previous;
				errors["OB_PROP_TEMPERATURE_COMPENSATION_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_MIRROR_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_depthMirror;
			if (!candidate.m_depthMirror)
				m_orbbecCtrl.m_depthMirror.reset();
			else if (!setDepthMirror(*candidate.m_depthMirror))
			{
				m_orbbecCtrl.m_depthMirror = previous;
				errors["OB_PROP_DEPTH_MIRROR_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_FLIP_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_depthFlip;
			if (!candidate.m_depthFlip)
				m_orbbecCtrl.m_depthFlip.reset();
			else if (!setDepthFlip(*candidate.m_depthFlip))
			{
				m_orbbecCtrl.m_depthFlip = previous;
				errors["OB_PROP_DEPTH_FLIP_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_HOLEFILTER_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_depthHolefilter;
			if (!candidate.m_depthHolefilter)
				m_orbbecCtrl.m_depthHolefilter.reset();
			else if (!setDepthHolefilter(*candidate.m_depthHolefilter))
			{
				m_orbbecCtrl.m_depthHolefilter = previous;
				errors["OB_PROP_DEPTH_HOLEFILTER_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_MIRROR_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_irMirror;
			if (!candidate.m_irMirror)
				m_orbbecCtrl.m_irMirror.reset();
			else if (!setIRMirror(*candidate.m_irMirror))
			{
				m_orbbecCtrl.m_irMirror = previous;
				errors["OB_PROP_IR_MIRROR_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_FLIP_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_irFlip;
			if (!candidate.m_irFlip)
				m_orbbecCtrl.m_irFlip.reset();
			else if (!setIRFlip(*candidate.m_irFlip))
			{
				m_orbbecCtrl.m_irFlip = previous;
				errors["OB_PROP_IR_FLIP_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_depthNoiseRemovalFilter;
			if (!candidate.m_depthNoiseRemovalFilter)
				m_orbbecCtrl.m_depthNoiseRemovalFilter.reset();
			else if (!setDepthNoiseRemovalFilter(*candidate.m_depthNoiseRemovalFilter))
			{
				m_orbbecCtrl.m_depthNoiseRemovalFilter = previous;
				errors["OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT"))
		{
			const auto previous = m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff;
			if (!candidate.m_depthNoiseRemovalFilterMaxDiff)
				m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff.reset();
			else if (!setDepthNoiseRemovalFilterMaxDiff(*candidate.m_depthNoiseRemovalFilterMaxDiff))
			{
				m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff = previous;
				errors["OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize;
			if (!candidate.m_depthNoiseRemovalFilterMaxSpeckleSize)
				m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize.reset();
			else if (!setDepthNoiseRemovalFilterMaxSpeckleSize(*candidate.m_depthNoiseRemovalFilterMaxSpeckleSize))
			{
				m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize = previous;
				errors["OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_depthAlignHardware;
			if (!candidate.m_depthAlignHardware)
				m_orbbecCtrl.m_depthAlignHardware.reset();
			else if (!setDepthAlignHardware(*candidate.m_depthAlignHardware))
			{
				m_orbbecCtrl.m_depthAlignHardware = previous;
				errors["OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_depthAlignHardwareMode;
			if (!candidate.m_depthAlignHardwareMode)
				m_orbbecCtrl.m_depthAlignHardwareMode.reset();
			else if (!setDepthAlignHardwareMode(*candidate.m_depthAlignHardwareMode))
			{
				m_orbbecCtrl.m_depthAlignHardwareMode = previous;
				errors["OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_PRECISION_LEVEL_INT"))
		{
			const auto previous = m_orbbecCtrl.m_depthPrecisionLevel;
			if (!candidate.m_depthPrecisionLevel)
				m_orbbecCtrl.m_depthPrecisionLevel.reset();
			else if (!setDepthPrecisionLevel(*candidate.m_depthPrecisionLevel))
			{
				m_orbbecCtrl.m_depthPrecisionLevel = previous;
				errors["OB_PROP_DEPTH_PRECISION_LEVEL_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_MIRROR_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorMirror;
			if (!candidate.m_colorMirror)
				m_orbbecCtrl.m_colorMirror.reset();
			else if (!setColorMirror(*candidate.m_colorMirror))
			{
				m_orbbecCtrl.m_colorMirror = previous;
				errors["OB_PROP_COLOR_MIRROR_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_FLIP_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorFlip;
			if (!candidate.m_colorFlip)
				m_orbbecCtrl.m_colorFlip.reset();
			else if (!setColorFlip(*candidate.m_colorFlip))
			{
				m_orbbecCtrl.m_colorFlip = previous;
				errors["OB_PROP_COLOR_FLIP_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DISPARITY_TO_DEPTH_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_disparityToDepth;
			if (!candidate.m_disparityToDepth)
				m_orbbecCtrl.m_disparityToDepth.reset();
			else if (!setDisparityToDepth(*candidate.m_disparityToDepth))
			{
				m_orbbecCtrl.m_disparityToDepth = previous;
				errors["OB_PROP_DISPARITY_TO_DEPTH_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_WATCHDOG_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_watchdog;
			if (!candidate.m_watchdog)
				m_orbbecCtrl.m_watchdog.reset();
			else if (!setWatchdog(*candidate.m_watchdog))
			{
				m_orbbecCtrl.m_watchdog = previous;
				errors["OB_PROP_WATCHDOG_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_externalSignalReset;
			if (!candidate.m_externalSignalReset)
				m_orbbecCtrl.m_externalSignalReset.reset();
			else if (!setExternalSignalReset(*candidate.m_externalSignalReset))
			{
				m_orbbecCtrl.m_externalSignalReset = previous;
				errors["OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_HEARTBEAT_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_heartbeat;
			if (!candidate.m_heartbeat)
				m_orbbecCtrl.m_heartbeat.reset();
			else if (!setHeartbeat(*candidate.m_heartbeat))
			{
				m_orbbecCtrl.m_heartbeat = previous;
				errors["OB_PROP_HEARTBEAT_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_LASER_POWER_LEVEL_CONTROL_INT"))
		{
			const auto previous = m_orbbecCtrl.m_laserPowerLevelControl;
			if (!candidate.m_laserPowerLevelControl)
				m_orbbecCtrl.m_laserPowerLevelControl.reset();
			else if (!setLaserPowerLevelControl(*candidate.m_laserPowerLevelControl))
			{
				m_orbbecCtrl.m_laserPowerLevelControl = previous;
				errors["OB_PROP_LASER_POWER_LEVEL_CONTROL_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_timerResetTriggerOutEnable;
			if (!candidate.m_timerResetTriggerOutEnable)
				m_orbbecCtrl.m_timerResetTriggerOutEnable.reset();
			else if (!setTimerResetTriggerOutEnable(*candidate.m_timerResetTriggerOutEnable))
			{
				m_orbbecCtrl.m_timerResetTriggerOutEnable = previous;
				errors["OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_TIMER_RESET_DELAY_US_INT"))
		{
			const auto previous = m_orbbecCtrl.m_timerResetDelayUs;
			if (!candidate.m_timerResetDelayUs)
				m_orbbecCtrl.m_timerResetDelayUs.reset();
			else if (!setTimerResetDelayUs(*candidate.m_timerResetDelayUs))
			{
				m_orbbecCtrl.m_timerResetDelayUs = previous;
				errors["OB_PROP_TIMER_RESET_DELAY_US_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_RIGHT_MIRROR_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_irRightMirror;
			if (!candidate.m_irRightMirror)
				m_orbbecCtrl.m_irRightMirror.reset();
			else if (!setIRRightMirror(*candidate.m_irRightMirror))
			{
				m_orbbecCtrl.m_irRightMirror = previous;
				errors["OB_PROP_IR_RIGHT_MIRROR_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT"))
		{
			const auto previous = m_orbbecCtrl.m_captureImageFrameNumber;
			if (!candidate.m_captureImageFrameNumber)
				m_orbbecCtrl.m_captureImageFrameNumber.reset();
			else if (!setCaptureImageFrameNumber(*candidate.m_captureImageFrameNumber))
			{
				m_orbbecCtrl.m_captureImageFrameNumber = previous;
				errors["OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_RIGHT_FLIP_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_irRightFlip;
			if (!candidate.m_irRightFlip)
				m_orbbecCtrl.m_irRightFlip.reset();
			else if (!setIRRightFlip(*candidate.m_irRightFlip))
			{
				m_orbbecCtrl.m_irRightFlip = previous;
				errors["OB_PROP_IR_RIGHT_FLIP_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_ROTATE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorRotate;
			if (!candidate.m_colorRotate)
				m_orbbecCtrl.m_colorRotate.reset();
			else if (!setColorRotate(*candidate.m_colorRotate))
			{
				m_orbbecCtrl.m_colorRotate = previous;
				errors["OB_PROP_COLOR_ROTATE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_ROTATE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_irRotate;
			if (!candidate.m_irRotate)
				m_orbbecCtrl.m_irRotate.reset();
			else if (!setIRRotate(*candidate.m_irRotate))
			{
				m_orbbecCtrl.m_irRotate = previous;
				errors["OB_PROP_IR_ROTATE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_RIGHT_ROTATE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_irRightRotate;
			if (!candidate.m_irRightRotate)
				m_orbbecCtrl.m_irRightRotate.reset();
			else if (!setIRRightRotate(*candidate.m_irRightRotate))
			{
				m_orbbecCtrl.m_irRightRotate = previous;
				errors["OB_PROP_IR_RIGHT_ROTATE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_ROTATE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_depthRotate;
			if (!candidate.m_depthRotate)
				m_orbbecCtrl.m_depthRotate.reset();
			else if (!setDepthRotate(*candidate.m_depthRotate))
			{
				m_orbbecCtrl.m_depthRotate = previous;
				errors["OB_PROP_DEPTH_ROTATE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_syncSignalTriggerOut;
			if (!candidate.m_syncSignalTriggerOut)
				m_orbbecCtrl.m_syncSignalTriggerOut.reset();
			else if (!setSyncSignalTriggerOut(*candidate.m_syncSignalTriggerOut))
			{
				m_orbbecCtrl.m_syncSignalTriggerOut = previous;
				errors["OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_deviceUSB2RepeatIdentify;
			if (!candidate.m_deviceUSB2RepeatIdentify)
				m_orbbecCtrl.m_deviceUSB2RepeatIdentify.reset();
			else if (!setDeviceUSB2RepeatIdentify(*candidate.m_deviceUSB2RepeatIdentify))
			{
				m_orbbecCtrl.m_deviceUSB2RepeatIdentify = previous;
				errors["OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_LASER_ALWAYS_ON_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_laserAlwaysOn;
			if (!candidate.m_laserAlwaysOn)
				m_orbbecCtrl.m_laserAlwaysOn.reset();
			else if (!setLaserAlwaysOn(*candidate.m_laserAlwaysOn))
			{
				m_orbbecCtrl.m_laserAlwaysOn = previous;
				errors["OB_PROP_LASER_ALWAYS_ON_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_LASER_ON_OFF_PATTERN_INT"))
		{
			const auto previous = m_orbbecCtrl.m_laserOnOffPattern;
			if (!candidate.m_laserOnOffPattern)
				m_orbbecCtrl.m_laserOnOffPattern.reset();
			else if (!setLaserOnOffPattern(*candidate.m_laserOnOffPattern))
			{
				m_orbbecCtrl.m_laserOnOffPattern = previous;
				errors["OB_PROP_LASER_ON_OFF_PATTERN_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT"))
		{
			const auto previous = m_orbbecCtrl.m_depthUnitFlexibleAdjustment;
			if (!candidate.m_depthUnitFlexibleAdjustment)
				m_orbbecCtrl.m_depthUnitFlexibleAdjustment.reset();
			else if (!setDepthUnitFlexibleAdjustment(*candidate.m_depthUnitFlexibleAdjustment))
			{
				m_orbbecCtrl.m_depthUnitFlexibleAdjustment = previous;
				errors["OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_LASER_CONTROL_INT"))
		{
			const auto previous = m_orbbecCtrl.m_laserControl;
			if (!candidate.m_laserControl)
				m_orbbecCtrl.m_laserControl.reset();
			else if (!setLaserControl(*candidate.m_laserControl))
			{
				m_orbbecCtrl.m_laserControl = previous;
				errors["OB_PROP_LASER_CONTROL_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_BRIGHTNESS_INT"))
		{
			const auto previous = m_orbbecCtrl.m_irBrightness;
			if (!candidate.m_irBrightness)
				m_orbbecCtrl.m_irBrightness.reset();
			else if (!setIRBrightness(*candidate.m_irBrightness))
			{
				m_orbbecCtrl.m_irBrightness = previous;
				errors["OB_PROP_IR_BRIGHTNESS_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_AE_MAX_EXPOSURE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorAEMaxExposure;
			if (!candidate.m_colorAEMaxExposure)
				m_orbbecCtrl.m_colorAEMaxExposure.reset();
			else if (!setColorAEMaxExposure(*candidate.m_colorAEMaxExposure))
			{
				m_orbbecCtrl.m_colorAEMaxExposure = previous;
				errors["OB_PROP_COLOR_AE_MAX_EXPOSURE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_AE_MAX_EXPOSURE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_irAEMaxExposure;
			if (!candidate.m_irAEMaxExposure)
				m_orbbecCtrl.m_irAEMaxExposure.reset();
			else if (!setIRAEMaxExposure(*candidate.m_irAEMaxExposure))
			{
				m_orbbecCtrl.m_irAEMaxExposure = previous;
				errors["OB_PROP_IR_AE_MAX_EXPOSURE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DISP_SEARCH_RANGE_MODE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_dispSearchRangeMode;
			if (!candidate.m_dispSearchRangeMode)
				m_orbbecCtrl.m_dispSearchRangeMode.reset();
			else if (!setDispSearchRangeMode(*candidate.m_dispSearchRangeMode))
			{
				m_orbbecCtrl.m_dispSearchRangeMode = previous;
				errors["OB_PROP_DISP_SEARCH_RANGE_MODE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DISP_SEARCH_OFFSET_INT"))
		{
			const auto previous = m_orbbecCtrl.m_dispSearchOffset;
			if (!candidate.m_dispSearchOffset)
				m_orbbecCtrl.m_dispSearchOffset.reset();
			else if (!setDispSearchOffset(*candidate.m_dispSearchOffset))
			{
				m_orbbecCtrl.m_dispSearchOffset = previous;
				errors["OB_PROP_DISP_SEARCH_OFFSET_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_cpuTemperatureCalibration;
			if (!candidate.m_cpuTemperatureCalibration)
				m_orbbecCtrl.m_cpuTemperatureCalibration.reset();
			else if (!setCPUTemperatureCalibration(*candidate.m_cpuTemperatureCalibration))
			{
				m_orbbecCtrl.m_cpuTemperatureCalibration = previous;
				errors["OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT"))
		{
			const auto previous = m_orbbecCtrl.m_frameInterleaveConfigIndex;
			if (!candidate.m_frameInterleaveConfigIndex)
				m_orbbecCtrl.m_frameInterleaveConfigIndex.reset();
			else if (!setFrameInterleaveConfigIndex(*candidate.m_frameInterleaveConfigIndex))
			{
				m_orbbecCtrl.m_frameInterleaveConfigIndex = previous;
				errors["OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_frameInterleaveEnable;
			if (!candidate.m_frameInterleaveEnable)
				m_orbbecCtrl.m_frameInterleaveEnable.reset();
			else if (!setFrameInterleaveEnable(*candidate.m_frameInterleaveEnable))
			{
				m_orbbecCtrl.m_frameInterleaveEnable = previous;
				errors["OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT"))
		{
			const auto previous = m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay;
			if (!candidate.m_frameInterleaveLaserPatternSyncDelay)
				m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay.reset();
			else if (!setFrameInterleaveLaserPatternSyncDelay(*candidate.m_frameInterleaveLaserPatternSyncDelay))
			{
				m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay = previous;
				errors["OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_onChipCalibrationEnable;
			if (!candidate.m_onChipCalibrationEnable)
				m_orbbecCtrl.m_onChipCalibrationEnable.reset();
			else if (!setOnChipCalibrationEnable(*candidate.m_onChipCalibrationEnable))
			{
				m_orbbecCtrl.m_onChipCalibrationEnable = previous;
				errors["OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_hwNoiseRemoveFilterEnable;
			if (!candidate.m_hwNoiseRemoveFilterEnable)
				m_orbbecCtrl.m_hwNoiseRemoveFilterEnable.reset();
			else if (!setHWNoiseRemoveFilterEnable(*candidate.m_hwNoiseRemoveFilterEnable))
			{
				m_orbbecCtrl.m_hwNoiseRemoveFilterEnable = previous;
				errors["OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT"))
		{
			const auto previous = m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold;
			if (!candidate.m_hwNoiseRemoveFilterThreshold)
				m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold.reset();
			else if (!setHWNoiseRemoveFilterThreshold(*candidate.m_hwNoiseRemoveFilterThreshold))
			{
				m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold = previous;
				errors["OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_deviceAutoCaptureEnable;
			if (!candidate.m_deviceAutoCaptureEnable)
				m_orbbecCtrl.m_deviceAutoCaptureEnable.reset();
			else if (!setDeviceAutoCaptureEnable(*candidate.m_deviceAutoCaptureEnable))
			{
				m_orbbecCtrl.m_deviceAutoCaptureEnable = previous;
				errors["OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT"))
		{
			const auto previous = m_orbbecCtrl.m_deviceAutoCaptureIntervalTime;
			if (!candidate.m_deviceAutoCaptureIntervalTime)
				m_orbbecCtrl.m_deviceAutoCaptureIntervalTime.reset();
			else if (!setDeviceAutoCaptureIntervalTime(*candidate.m_deviceAutoCaptureIntervalTime))
			{
				m_orbbecCtrl.m_deviceAutoCaptureIntervalTime = previous;
				errors["OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_devicePTPClockSyncEnable;
			if (!candidate.m_devicePTPClockSyncEnable)
				m_orbbecCtrl.m_devicePTPClockSyncEnable.reset();
			else if (!setDevicePTPClockSyncEnable(*candidate.m_devicePTPClockSyncEnable))
			{
				m_orbbecCtrl.m_devicePTPClockSyncEnable = previous;
				errors["OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_confidenceStreamFilter;
			if (!candidate.m_confidenceStreamFilter)
				m_orbbecCtrl.m_confidenceStreamFilter.reset();
			else if (!setConfidenceStreamFilter(*candidate.m_confidenceStreamFilter))
			{
				m_orbbecCtrl.m_confidenceStreamFilter = previous;
				errors["OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT"))
		{
			const auto previous = m_orbbecCtrl.m_confidenceStreamFilterThreshold;
			if (!candidate.m_confidenceStreamFilterThreshold)
				m_orbbecCtrl.m_confidenceStreamFilterThreshold.reset();
			else if (!setConfidenceStreamFilterThreshold(*candidate.m_confidenceStreamFilterThreshold))
			{
				m_orbbecCtrl.m_confidenceStreamFilterThreshold = previous;
				errors["OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_CONFIDENCE_MIRROR_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_confidenceMirror;
			if (!candidate.m_confidenceMirror)
				m_orbbecCtrl.m_confidenceMirror.reset();
			else if (!setConfidenceMirror(*candidate.m_confidenceMirror))
			{
				m_orbbecCtrl.m_confidenceMirror = previous;
				errors["OB_PROP_CONFIDENCE_MIRROR_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_CONFIDENCE_FLIP_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_confidenceFlip;
			if (!candidate.m_confidenceFlip)
				m_orbbecCtrl.m_confidenceFlip.reset();
			else if (!setConfidenceFlip(*candidate.m_confidenceFlip))
			{
				m_orbbecCtrl.m_confidenceFlip = previous;
				errors["OB_PROP_CONFIDENCE_FLIP_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_CONFIDENCE_ROTATE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_confidenceRotate;
			if (!candidate.m_confidenceRotate)
				m_orbbecCtrl.m_confidenceRotate.reset();
			else if (!setConfidenceRotate(*candidate.m_confidenceRotate))
			{
				m_orbbecCtrl.m_confidenceRotate = previous;
				errors["OB_PROP_CONFIDENCE_ROTATE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_intraCameraSyncReference;
			if (!candidate.m_intraCameraSyncReference)
				m_orbbecCtrl.m_intraCameraSyncReference.reset();
			else if (!setIntraCameraSyncReference(*candidate.m_intraCameraSyncReference))
			{
				m_orbbecCtrl.m_intraCameraSyncReference = previous;
				errors["OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_RIGHT_ROTATE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorRightRotate;
			if (!candidate.m_colorRightRotate)
				m_orbbecCtrl.m_colorRightRotate.reset();
			else if (!setColorRightRotate(*candidate.m_colorRightRotate))
			{
				m_orbbecCtrl.m_colorRightRotate = previous;
				errors["OB_PROP_COLOR_RIGHT_ROTATE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_RIGHT_MIRROR_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorRightMirror;
			if (!candidate.m_colorRightMirror)
				m_orbbecCtrl.m_colorRightMirror.reset();
			else if (!setColorRightMirror(*candidate.m_colorRightMirror))
			{
				m_orbbecCtrl.m_colorRightMirror = previous;
				errors["OB_PROP_COLOR_RIGHT_MIRROR_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_RIGHT_FLIP_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorRightFlip;
			if (!candidate.m_colorRightFlip)
				m_orbbecCtrl.m_colorRightFlip.reset();
			else if (!setColorRightFlip(*candidate.m_colorRightFlip))
			{
				m_orbbecCtrl.m_colorRightFlip = previous;
				errors["OB_PROP_COLOR_RIGHT_FLIP_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEVICE_AE_REFERENCE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_deviceAEReference;
			if (!candidate.m_deviceAEReference)
				m_orbbecCtrl.m_deviceAEReference.reset();
			else if (!setDeviceAEReference(*candidate.m_deviceAEReference))
			{
				m_orbbecCtrl.m_deviceAEReference = previous;
				errors["OB_PROP_DEVICE_AE_REFERENCE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEVICE_AE_STRATEGY_INT"))
		{
			const auto previous = m_orbbecCtrl.m_deviceAEStrategy;
			if (!candidate.m_deviceAEStrategy)
				m_orbbecCtrl.m_deviceAEStrategy.reset();
			else if (!setDeviceAEStrategy(*candidate.m_deviceAEStrategy))
			{
				m_orbbecCtrl.m_deviceAEStrategy = previous;
				errors["OB_PROP_DEVICE_AE_STRATEGY_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_ROI_BRIGHTNESS_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorROIBrightness;
			if (!candidate.m_colorROIBrightness)
				m_orbbecCtrl.m_colorROIBrightness.reset();
			else if (!setColorROIBrightness(*candidate.m_colorROIBrightness))
			{
				m_orbbecCtrl.m_colorROIBrightness = previous;
				errors["OB_PROP_COLOR_ROI_BRIGHTNESS_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_LEFT_ROTATE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorLeftRotate;
			if (!candidate.m_colorLeftRotate)
				m_orbbecCtrl.m_colorLeftRotate.reset();
			else if (!setColorLeftRotate(*candidate.m_colorLeftRotate))
			{
				m_orbbecCtrl.m_colorLeftRotate = previous;
				errors["OB_PROP_COLOR_LEFT_ROTATE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_LEFT_MIRROR_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorLeftMirror;
			if (!candidate.m_colorLeftMirror)
				m_orbbecCtrl.m_colorLeftMirror.reset();
			else if (!setColorLeftMirror(*candidate.m_colorLeftMirror))
			{
				m_orbbecCtrl.m_colorLeftMirror = previous;
				errors["OB_PROP_COLOR_LEFT_MIRROR_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_LEFT_FLIP_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorLeftFlip;
			if (!candidate.m_colorLeftFlip)
				m_orbbecCtrl.m_colorLeftFlip.reset();
			else if (!setColorLeftFlip(*candidate.m_colorLeftFlip))
			{
				m_orbbecCtrl.m_colorLeftFlip = previous;
				errors["OB_PROP_COLOR_LEFT_FLIP_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_PRESET_PRIORITY_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorPresetPriority;
			if (!candidate.m_colorPresetPriority)
				m_orbbecCtrl.m_colorPresetPriority.reset();
			else if (!setColorPresetPriority(*candidate.m_colorPresetPriority))
			{
				m_orbbecCtrl.m_colorPresetPriority = previous;
				errors["OB_PROP_COLOR_PRESET_PRIORITY_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEVICE_NETWORK_LLA_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_deviceNetworkLLA;
			if (!candidate.m_deviceNetworkLLA)
				m_orbbecCtrl.m_deviceNetworkLLA.reset();
			else if (!setDeviceNetworkLLA(*candidate.m_deviceNetworkLLA))
			{
				m_orbbecCtrl.m_deviceNetworkLLA = previous;
				errors["OB_PROP_DEVICE_NETWORK_LLA_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_ANTI_FLICKER_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorAntiFlicker;
			if (!candidate.m_colorAntiFlicker)
				m_orbbecCtrl.m_colorAntiFlicker.reset();
			else if (!setColorAntiFlicker(*candidate.m_colorAntiFlicker))
			{
				m_orbbecCtrl.m_colorAntiFlicker = previous;
				errors["OB_PROP_COLOR_ANTI_FLICKER_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEVICE_IP_MODE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_deviceIPMode;
			if (!candidate.m_deviceIPMode)
				m_orbbecCtrl.m_deviceIPMode.reset();
			else if (!setDeviceIPMode(*candidate.m_deviceIPMode))
			{
				m_orbbecCtrl.m_deviceIPMode = previous;
				errors["OB_PROP_DEVICE_IP_MODE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT"))
		{
			const auto previous = m_orbbecCtrl.m_dhcpAssignIPTimeout;
			if (!candidate.m_dhcpAssignIPTimeout)
				m_orbbecCtrl.m_dhcpAssignIPTimeout.reset();
			else if (!setDHCPAssignIPTimeout(*candidate.m_dhcpAssignIPTimeout))
			{
				m_orbbecCtrl.m_dhcpAssignIPTimeout = previous;
				errors["OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT"))
		{
			const auto previous = m_orbbecCtrl.m_usbSyncVoltageLevel;
			if (!candidate.m_usbSyncVoltageLevel)
				m_orbbecCtrl.m_usbSyncVoltageLevel.reset();
			else if (!setUSBSyncVoltageLevel(*candidate.m_usbSyncVoltageLevel))
			{
				m_orbbecCtrl.m_usbSyncVoltageLevel = previous;
				errors["OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_FPS_BOOST_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_fpsBoost;
			if (!candidate.m_fpsBoost)
				m_orbbecCtrl.m_fpsBoost.reset();
			else if (!setFPSBoost(*candidate.m_fpsBoost))
			{
				m_orbbecCtrl.m_fpsBoost = previous;
				errors["OB_PROP_FPS_BOOST_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_MJPEG_QUALITY_INT"))
		{
			const auto previous = m_orbbecCtrl.m_mjpegQuality;
			if (!candidate.m_mjpegQuality)
				m_orbbecCtrl.m_mjpegQuality.reset();
			else if (!setMJPEGQuality(*candidate.m_mjpegQuality))
			{
				m_orbbecCtrl.m_mjpegQuality = previous;
				errors["OB_PROP_MJPEG_QUALITY_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_AUTO_EXPOSURE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorAutoExposure;
			if (!candidate.m_colorAutoExposure)
				m_orbbecCtrl.m_colorAutoExposure.reset();
			else if (!setColorAutoExposure(*candidate.m_colorAutoExposure))
			{
				m_orbbecCtrl.m_colorAutoExposure = previous;
				errors["OB_PROP_COLOR_AUTO_EXPOSURE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_EXPOSURE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorExposure;
			if (!candidate.m_colorExposure)
				m_orbbecCtrl.m_colorExposure.reset();
			else if (!setColorExposure(*candidate.m_colorExposure))
			{
				m_orbbecCtrl.m_colorExposure = previous;
				errors["OB_PROP_COLOR_EXPOSURE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_GAIN_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorGain;
			if (!candidate.m_colorGain)
				m_orbbecCtrl.m_colorGain.reset();
			else if (!setColorGain(*candidate.m_colorGain))
			{
				m_orbbecCtrl.m_colorGain = previous;
				errors["OB_PROP_COLOR_GAIN_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_colorAutoWhiteBalance;
			if (!candidate.m_colorAutoWhiteBalance)
				m_orbbecCtrl.m_colorAutoWhiteBalance.reset();
			else if (!setColorAutoWhiteBalance(*candidate.m_colorAutoWhiteBalance))
			{
				m_orbbecCtrl.m_colorAutoWhiteBalance = previous;
				errors["OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_WHITE_BALANCE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorWhiteBalance;
			if (!candidate.m_colorWhiteBalance)
				m_orbbecCtrl.m_colorWhiteBalance.reset();
			else if (!setColorWhiteBalance(*candidate.m_colorWhiteBalance))
			{
				m_orbbecCtrl.m_colorWhiteBalance = previous;
				errors["OB_PROP_COLOR_WHITE_BALANCE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_BRIGHTNESS_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorBrightness;
			if (!candidate.m_colorBrightness)
				m_orbbecCtrl.m_colorBrightness.reset();
			else if (!setColorBrightness(*candidate.m_colorBrightness))
			{
				m_orbbecCtrl.m_colorBrightness = previous;
				errors["OB_PROP_COLOR_BRIGHTNESS_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_SHARPNESS_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorSharpness;
			if (!candidate.m_colorSharpness)
				m_orbbecCtrl.m_colorSharpness.reset();
			else if (!setColorSharpness(*candidate.m_colorSharpness))
			{
				m_orbbecCtrl.m_colorSharpness = previous;
				errors["OB_PROP_COLOR_SHARPNESS_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_SATURATION_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorSaturation;
			if (!candidate.m_colorSaturation)
				m_orbbecCtrl.m_colorSaturation.reset();
			else if (!setColorSaturation(*candidate.m_colorSaturation))
			{
				m_orbbecCtrl.m_colorSaturation = previous;
				errors["OB_PROP_COLOR_SATURATION_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_CONTRAST_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorContrast;
			if (!candidate.m_colorContrast)
				m_orbbecCtrl.m_colorContrast.reset();
			else if (!setColorContrast(*candidate.m_colorContrast))
			{
				m_orbbecCtrl.m_colorContrast = previous;
				errors["OB_PROP_COLOR_CONTRAST_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_GAMMA_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorGamma;
			if (!candidate.m_colorGamma)
				m_orbbecCtrl.m_colorGamma.reset();
			else if (!setColorGamma(*candidate.m_colorGamma))
			{
				m_orbbecCtrl.m_colorGamma = previous;
				errors["OB_PROP_COLOR_GAMMA_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorAutoExposurePriority;
			if (!candidate.m_colorAutoExposurePriority)
				m_orbbecCtrl.m_colorAutoExposurePriority.reset();
			else if (!setColorAutoExposurePriority(*candidate.m_colorAutoExposurePriority))
			{
				m_orbbecCtrl.m_colorAutoExposurePriority = previous;
				errors["OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorBacklightCompensation;
			if (!candidate.m_colorBacklightCompensation)
				m_orbbecCtrl.m_colorBacklightCompensation.reset();
			else if (!setColorBacklightCompensation(*candidate.m_colorBacklightCompensation))
			{
				m_orbbecCtrl.m_colorBacklightCompensation = previous;
				errors["OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_HUE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorHue;
			if (!candidate.m_colorHue)
				m_orbbecCtrl.m_colorHue.reset();
			else if (!setColorHue(*candidate.m_colorHue))
			{
				m_orbbecCtrl.m_colorHue = previous;
				errors["OB_PROP_COLOR_HUE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorPowerLineFrequency;
			if (!candidate.m_colorPowerLineFrequency)
				m_orbbecCtrl.m_colorPowerLineFrequency.reset();
			else if (!setColorPowerLineFrequency(*candidate.m_colorPowerLineFrequency))
			{
				m_orbbecCtrl.m_colorPowerLineFrequency = previous;
				errors["OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_depthAutoExposure;
			if (!candidate.m_depthAutoExposure)
				m_orbbecCtrl.m_depthAutoExposure.reset();
			else if (!setDepthAutoExposure(*candidate.m_depthAutoExposure))
			{
				m_orbbecCtrl.m_depthAutoExposure = previous;
				errors["OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_EXPOSURE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_depthExposure;
			if (!candidate.m_depthExposure)
				m_orbbecCtrl.m_depthExposure.reset();
			else if (!setDepthExposure(*candidate.m_depthExposure))
			{
				m_orbbecCtrl.m_depthExposure = previous;
				errors["OB_PROP_DEPTH_EXPOSURE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_GAIN_INT"))
		{
			const auto previous = m_orbbecCtrl.m_depthGain;
			if (!candidate.m_depthGain)
				m_orbbecCtrl.m_depthGain.reset();
			else if (!setDepthGain(*candidate.m_depthGain))
			{
				m_orbbecCtrl.m_depthGain = previous;
				errors["OB_PROP_DEPTH_GAIN_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_AUTO_EXPOSURE_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_irAutoExposure;
			if (!candidate.m_irAutoExposure)
				m_orbbecCtrl.m_irAutoExposure.reset();
			else if (!setIRAutoExposure(*candidate.m_irAutoExposure))
			{
				m_orbbecCtrl.m_irAutoExposure = previous;
				errors["OB_PROP_IR_AUTO_EXPOSURE_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_EXPOSURE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_irExposure;
			if (!candidate.m_irExposure)
				m_orbbecCtrl.m_irExposure.reset();
			else if (!setIRExposure(*candidate.m_irExposure))
			{
				m_orbbecCtrl.m_irExposure = previous;
				errors["OB_PROP_IR_EXPOSURE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_GAIN_INT"))
		{
			const auto previous = m_orbbecCtrl.m_irGain;
			if (!candidate.m_irGain)
				m_orbbecCtrl.m_irGain.reset();
			else if (!setIRGain(*candidate.m_irGain))
			{
				m_orbbecCtrl.m_irGain = previous;
				errors["OB_PROP_IR_GAIN_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_IR_CHANNEL_DATA_SOURCE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_irChannelDataSource;
			if (!candidate.m_irChannelDataSource)
				m_orbbecCtrl.m_irChannelDataSource.reset();
			else if (!setIRChannelDataSource(*candidate.m_irChannelDataSource))
			{
				m_orbbecCtrl.m_irChannelDataSource = previous;
				errors["OB_PROP_IR_CHANNEL_DATA_SOURCE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_RM_FILTER_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_depthRMFilter;
			if (!candidate.m_depthRMFilter)
				m_orbbecCtrl.m_depthRMFilter.reset();
			else if (!setDepthRMFilter(*candidate.m_depthRMFilter))
			{
				m_orbbecCtrl.m_depthRMFilter = previous;
				errors["OB_PROP_DEPTH_RM_FILTER_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_AE_MAX_GAIN_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorAEMaxGain;
			if (!candidate.m_colorAEMaxGain)
				m_orbbecCtrl.m_colorAEMaxGain.reset();
			else if (!setColorAEMaxGain(*candidate.m_colorAEMaxGain))
			{
				m_orbbecCtrl.m_colorAEMaxGain = previous;
				errors["OB_PROP_COLOR_AE_MAX_GAIN_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT"))
		{
			const auto previous = m_orbbecCtrl.m_depthAutoExposurePriority;
			if (!candidate.m_depthAutoExposurePriority)
				m_orbbecCtrl.m_depthAutoExposurePriority.reset();
			else if (!setDepthAutoExposurePriority(*candidate.m_depthAutoExposurePriority))
			{
				m_orbbecCtrl.m_depthAutoExposurePriority = previous;
				errors["OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_sdkDisparityToDepth;
			if (!candidate.m_sdkDisparityToDepth)
				m_orbbecCtrl.m_sdkDisparityToDepth.reset();
			else if (!setSDKDisparityToDepth(*candidate.m_sdkDisparityToDepth))
			{
				m_orbbecCtrl.m_sdkDisparityToDepth = previous;
				errors["OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_sdkAccelFrameTransformed;
			if (!candidate.m_sdkAccelFrameTransformed)
				m_orbbecCtrl.m_sdkAccelFrameTransformed.reset();
			else if (!setSDKAccelFrameTransformed(*candidate.m_sdkAccelFrameTransformed))
			{
				m_orbbecCtrl.m_sdkAccelFrameTransformed = previous;
				errors["OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL"))
		{
			const auto previous = m_orbbecCtrl.m_sdkGyroFrameTransformed;
			if (!candidate.m_sdkGyroFrameTransformed)
				m_orbbecCtrl.m_sdkGyroFrameTransformed.reset();
			else if (!setSDKGyroFrameTransformed(*candidate.m_sdkGyroFrameTransformed))
			{
				m_orbbecCtrl.m_sdkGyroFrameTransformed = previous;
				errors["OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_DEVICE_PERFORMANCE_MODE_INT"))
		{
			const auto previous = m_orbbecCtrl.m_devicePerformanceMode;
			if (!candidate.m_devicePerformanceMode)
				m_orbbecCtrl.m_devicePerformanceMode.reset();
			else if (!setDevicePerformanceMode(*candidate.m_devicePerformanceMode))
			{
				m_orbbecCtrl.m_devicePerformanceMode = previous;
				errors["OB_PROP_DEVICE_PERFORMANCE_MODE_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_PROP_COLOR_DENOISING_LEVEL_INT"))
		{
			const auto previous = m_orbbecCtrl.m_colorDenoisingLevel;
			if (!candidate.m_colorDenoisingLevel)
				m_orbbecCtrl.m_colorDenoisingLevel.reset();
			else if (!setColorDenoisingLevel(*candidate.m_colorDenoisingLevel))
			{
				m_orbbecCtrl.m_colorDenoisingLevel = previous;
				errors["OB_PROP_COLOR_DENOISING_LEVEL_INT"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_STRUCT_MULTI_DEVICE_SYNC_CONFIG"))
		{
			const auto previous = m_orbbecCtrl.m_multiDeviceSyncConfig;
			if (!candidate.m_multiDeviceSyncConfig)
				m_orbbecCtrl.m_multiDeviceSyncConfig.reset();
			else if (!setMultiDeviceSyncConfig(*candidate.m_multiDeviceSyncConfig))
			{
				m_orbbecCtrl.m_multiDeviceSyncConfig = previous;
				errors["OB_STRUCT_MULTI_DEVICE_SYNC_CONFIG"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_STRUCT_DEVICE_IP_ADDR_CONFIG"))
		{
			const auto previous = m_orbbecCtrl.m_deviceIPAddrConfig;
			if (!candidate.m_deviceIPAddrConfig)
				m_orbbecCtrl.m_deviceIPAddrConfig.reset();
			else if (!setDeviceIPAddrConfig(*candidate.m_deviceIPAddrConfig))
			{
				m_orbbecCtrl.m_deviceIPAddrConfig = previous;
				errors["OB_STRUCT_DEVICE_IP_ADDR_CONFIG"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_STRUCT_DEPTH_HDR_CONFIG"))
		{
			const auto previous = m_orbbecCtrl.m_depthHdrConfig;
			if (!candidate.m_depthHdrConfig)
				m_orbbecCtrl.m_depthHdrConfig.reset();
			else if (!setDepthHdrConfig(*candidate.m_depthHdrConfig))
			{
				m_orbbecCtrl.m_depthHdrConfig = previous;
				errors["OB_STRUCT_DEPTH_HDR_CONFIG"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_STRUCT_COLOR_AE_ROI"))
		{
			const auto previous = m_orbbecCtrl.m_colorAEROI;
			if (!candidate.m_colorAEROI)
				m_orbbecCtrl.m_colorAEROI.reset();
			else if (!setColorAEROI(*candidate.m_colorAEROI))
			{
				m_orbbecCtrl.m_colorAEROI = previous;
				errors["OB_STRUCT_COLOR_AE_ROI"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_STRUCT_DEPTH_AE_ROI"))
		{
			const auto previous = m_orbbecCtrl.m_depthAEROI;
			if (!candidate.m_depthAEROI)
				m_orbbecCtrl.m_depthAEROI.reset();
			else if (!setDepthAEROI(*candidate.m_depthAEROI))
			{
				m_orbbecCtrl.m_depthAEROI = previous;
				errors["OB_STRUCT_DEPTH_AE_ROI"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_STRUCT_DISP_OFFSET_CONFIG"))
		{
			const auto previous = m_orbbecCtrl.m_dispOffsetConfig;
			if (!candidate.m_dispOffsetConfig)
				m_orbbecCtrl.m_dispOffsetConfig.reset();
			else if (!setDispOffsetConfig(*candidate.m_dispOffsetConfig))
			{
				m_orbbecCtrl.m_dispOffsetConfig = previous;
				errors["OB_STRUCT_DISP_OFFSET_CONFIG"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_STRUCT_PRESET_RESOLUTION_CONFIG"))
		{
			const auto previous = m_orbbecCtrl.m_presetResolutionConfig;
			if (!candidate.m_presetResolutionConfig)
				m_orbbecCtrl.m_presetResolutionConfig.reset();
			else if (!setPresetResolutionConfig(*candidate.m_presetResolutionConfig))
			{
				m_orbbecCtrl.m_presetResolutionConfig = previous;
				errors["OB_STRUCT_PRESET_RESOLUTION_CONFIG"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2"))
		{
			const auto previous = m_orbbecCtrl.m_deviceIPAddrConfigV2;
			if (!candidate.m_deviceIPAddrConfigV2)
				m_orbbecCtrl.m_deviceIPAddrConfigV2.reset();
			else if (!setDeviceIPAddrConfigV2(*candidate.m_deviceIPAddrConfigV2))
			{
				m_orbbecCtrl.m_deviceIPAddrConfigV2 = previous;
				errors["OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("obTimestampResetConfig"))
		{
			const auto previous = m_orbbecCtrl.m_timestampResetConfig;
			if (!candidate.m_timestampResetConfig)
				m_orbbecCtrl.m_timestampResetConfig.reset();
			else if (!setTimestampResetConfig(*candidate.m_timestampResetConfig))
			{
				m_orbbecCtrl.m_timestampResetConfig = previous;
				errors["obTimestampResetConfig"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("obGlobalTimestamp"))
		{
			const auto previous = m_orbbecCtrl.m_globalTimestamp;
			if (!candidate.m_globalTimestamp)
				m_orbbecCtrl.m_globalTimestamp.reset();
			else if (!enableGlobalTimestamp(*candidate.m_globalTimestamp))
			{
				m_orbbecCtrl.m_globalTimestamp = previous;
				errors["obGlobalTimestamp"] = "Device rejected this value or does not support this control";
			}
		}
		if (values.contains("obFirmwareLog"))
		{
			const auto previous = m_orbbecCtrl.m_firmwareLog;
			if (!candidate.m_firmwareLog)
				m_orbbecCtrl.m_firmwareLog.reset();
			else if (!enableFirmwareLog(*candidate.m_firmwareLog))
			{
				m_orbbecCtrl.m_firmwareLog = previous;
				errors["obFirmwareLog"] = "Device rejected this value or does not support this control";
			}
		}

		return errors.empty();
	}

	bool _Orbbec::saveConfig(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		if (!_RGBDbase::saveConfig())
		{
			return false;
		}

		const json current = configValues();
		for (const auto &field : current.items())
		{
			if (field.value().is_null())
			{
				m_pJ->erase(field.key());
				continue;
			}
			(*m_pJ)[field.key()] = field.value();
		}

		if (m_pTpp && !m_pTpp->saveConfig())
		{
			return false;
		}

		return m_pJcfg->saveToFile();
	}

	bool _Orbbec::open(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		IF__(m_bOpened, true);

		try
		{
			m_ctx.enableNetDeviceEnumeration(m_bNetDevEnum);
			shared_ptr<ob::DeviceList> pDL = m_ctx.queryDeviceList();
			NULL_F(pDL);
			IF_F(pDL->getCount() == 0);
			if (m_SN.empty())
			{
				m_spDev = pDL->getDevice(0);
			}
			else
			{
				m_spDev = pDL->getDeviceBySN(m_SN.c_str());
			}

			// Apply configured device options before starting any streams.
			// Unsupported model-specific options are reported by the setters.
			if (!setCamCtrl(m_orbbecCtrl))
				LOG_I("One or more configured Orbbec controls could not be applied");

			m_spSensorList = m_spDev->getSensorList();
			for (int i = 0; i < m_spSensorList->getCount(); i++)
			{
				OBSensorType sensorType = m_spSensorList->getSensorType(i);
				LOG_I("Supported Sensor type: " + i2str(sensorType));
			}

			// Pipeline
			m_spPipe = make_shared<ob::Pipeline>(m_spDev);
			m_spConfig = make_shared<ob::Config>();

			// RGBD streams
			if (m_bRGB)
				m_spConfig->enableVideoStream(OB_STREAM_COLOR, m_vSizeRGB.x(), m_vSizeRGB.y(), m_devFPS, OB_FORMAT_BGR);
			if (m_bDepth)
				m_spConfig->enableVideoStream(OB_STREAM_DEPTH, m_vSizeD.x(), m_vSizeD.y(), m_devFPSd, OB_FORMAT_Y16);

			// For point cloud generation
			if (m_bPCLrgb)
			{
				m_spConfig->setFrameAggregateOutputMode(OB_FRAME_AGGREGATE_OUTPUT_ALL_TYPE_FRAME_REQUIRE);
				m_spConfig->setAlignMode(ALIGN_D2C_HW_MODE);
				m_spPipe->enableFrameSync();
			}

			if (m_bRGB || m_bDepth)
				m_spPipe->start(m_spConfig);
			else
				m_spPipe.reset(); // IMU-only operation needs no video pipeline.

			// Video framesets retain only one IMU sample per video exposure.
			// Separate callbacks deliver every sample, even during slow cloud conversion.
			if (m_bIMU)
			{
				m_spAccel = m_spDev->getSensor(OB_SENSOR_ACCEL);
				m_spGyro = m_spDev->getSensor(OB_SENSOR_GYRO);
				auto accel = m_spAccel->getStreamProfileList()->getAccelStreamProfile(OB_ACCEL_FS_4g, OB_SAMPLE_RATE_200_HZ);
				auto gyro = m_spGyro->getStreamProfileList()->getGyroStreamProfile(OB_GYRO_FS_1000dps, OB_SAMPLE_RATE_200_HZ);
				// No device mutex here: Sensor::stop waits for callbacks to finish.
				m_spAccel->start(accel, [imu = m_pIMU](shared_ptr<ob::Frame> frame) {
					if (!imu || !frame) return;
					const auto v = frame->as<ob::AccelFrame>()->value(); // m/s^2
					imu->addAcc({v.x, v.y, v.z}, frame->getTimeStampUs() * NSEC_USEC);
				});
				m_spGyro->start(gyro, [imu = m_pIMU](shared_ptr<ob::Frame> frame) {
					if (!imu || !frame) return;
					const auto v = frame->as<ob::GyroFrame>()->value(); // rad/s
					imu->addGyro({v.x, v.y, v.z}, frame->getTimeStampUs() * NSEC_USEC);
				});
			}

			// Point cloud filter
			m_spPCF = std::make_shared<ob::PointCloudFilter>();
			//		m_spPCF->setCameraParam(m_spPipe->getCameraParam());
			if (m_bPCLrgb)
			{
				m_spPCF->setCreatePointFormat(OB_FORMAT_RGB_POINT);
			}
			else if (m_bPCL)
			{
				m_spPCF->setCreatePointFormat(OB_FORMAT_POINT);
			}

			m_tDNs = 0;
			m_dtDNs = 0;
			m_tRGBNs = 0;
			m_dtRGBNs = 0;

			m_bOpened = true;
			return true;
		}
		catch (const ob::Error &e)
		{
			LOG_E(e.what());
			close();
			return false;
		}
	}

	void _Orbbec::close(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		for (auto &sensor : {m_spAccel, m_spGyro})
			if (sensor) try { sensor->stop(); } catch (const ob::Error &e) { LOG_I(e.what()); }
		m_spAccel.reset();
		m_spGyro.reset();
		if (m_spPipe)
		{
			try { m_spPipe->stop(); } catch (const ob::Error &e) { LOG_I(e.what()); }
		}
		m_spPipe.reset();
		m_spDev.reset();
		m_spPCLframe.reset();
		m_bOpened = false;
		this->_RGBDbase::close();
	}

	bool _Orbbec::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTpp);

		IF_F(!m_pT->startThread(getUpdate, this));
		return m_pTpp->startThread(getTPP, this);
	}

	void _Orbbec::stop(void)
	{
		if (m_pT) m_pT->join();
		if (m_pTpp) m_pTpp->join();
		// Join SDK callbacks before ModuleMgr can release the linked IMU buffer.
		close();
	}

	bool _Orbbec::check(void)
	{
		return this->_RGBDbase::check();
	}

	void _Orbbec::update(void)
	{
		while (m_pT->bRun())
		{
			if (!open())
			{
				LOG_E("Cannot open Orbbec");
				m_pT->sleepT(NSEC_SEC);
				continue;
			}

			m_pT->autoFPS();

			if (updateOrbbec())
				m_pTpp->run();
			else
				close();
		}
	}

	bool _Orbbec::updateOrbbec(void)
	{
		shared_ptr<ob::Pipeline> pipeline;
		uint32_t timeout;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
			IF_F(!check() || !m_bOpened);
			if (!m_bRGB && !m_bDepth) return true;
			IF_F(!m_spPipe);
			pipeline = m_spPipe;
			timeout = m_tOutMs;
		}

		// Do not hold the device mutex while waiting for the next frame: doing so
		// can starve command handling when capture continuously waits at 30 Hz.
		shared_ptr<ob::FrameSet> spFS;
		try { spFS = pipeline->waitForFrameset(timeout); }
		catch (const ob::Error &e)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
			if (pipeline != m_spPipe) return true; // Stream changed during the wait.
			LOG_E(e.what());
			return false;
		}
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		if (pipeline != m_spPipe) return true;
		NULL_F(spFS);

		// Images, slow stream
		shared_ptr<ob::Frame> spFrameRGB = nullptr;
		shared_ptr<ob::Frame> spFrameD = nullptr;

		if (m_bRGB)
		{
			spFrameRGB = spFS->getFrame(OB_FRAME_COLOR);
			if (spFrameRGB)
			{
				// Own the pixels before the SDK frame is released, and exclude readers.
				{
					std::lock_guard<std::mutex> lock(m_mutexRGB);
					Mat(m_vSizeRGB.y(), m_vSizeRGB.x(), CV_8UC3, spFrameRGB->getData()).copyTo(m_mRGB);
				}
				uint64_t tRGBNs = frameTsNs(spFrameRGB);
				m_dtRGBNs = tRGBNs - m_tRGBNs;
				m_tRGBNs = tRGBNs;
			}
		}

		if (m_bDepth)
		{
			spFrameD = spFS->getFrame(OB_FRAME_DEPTH);
			if (spFrameD)
			{
				{
					std::lock_guard<std::mutex> lock(m_mutexDepth);
					Mat(m_vSizeD.y(), m_vSizeD.x(), CV_16UC1, spFrameD->getData()).copyTo(m_mDepth);
				}
				uint64_t tDNs = frameTsNs(spFrameD);
				m_dtDNs = tDNs - m_tDNs;
				m_tDNs = tDNs;
			}
		}

		// Capture must not wait for point conversion or per-point insertion.
		// Replacing this slot drops obsolete work when the cloud worker is slower.
#ifdef WITH_UNIVERSE
		if (m_pPCL && ((m_bPCLrgb && spFrameRGB && spFrameD) ||
			(!m_bPCLrgb && m_bPCL && spFrameD)))
			m_spPCLframe = spFS;
#endif

		return true;
	}

	void _Orbbec::updateTPP(void)
	{
		while (m_pTpp->bRun())
		{
			// A bounded wait also consumes a frame queued while the worker was
			// busy, even when its wakeup arrived before this iteration.
			m_pTpp->autoFPS();
			updatePCL();
		}
	}

	void _Orbbec::updatePCL(void)
	{
#ifdef WITH_UNIVERSE
		shared_ptr<ob::FrameSet> frames;
		shared_ptr<ob::PointCloudFilter> filter;
		_PointCloud *points;
		float scale;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
			frames = std::move(m_spPCLframe);
			filter = m_spPCF;
			points = m_pPCL;
			scale = m_dScale;
		}
		if (!frames || !filter || !points) return;

		// The shared pointers keep this job alive if capture restarts. Neither the
		// SDK filter nor cloud insertion may hold the camera's device mutex.
		shared_ptr<ob::Frame> spFrame;
		try { spFrame = filter->process(frames); }
		catch (const ob::Error &e) { LOG_E(e.what()); return; }
		if (!spFrame) return;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
			if (!m_bOpened || filter != m_spPCF) return;
		}

		const auto format = spFrame->getFormat();
		IF_(format != OB_FORMAT_POINT && format != OB_FORMAT_RGB_POINT);

		// The SDK scale converts point coordinates to millimeters.
		const float s_b = spFrame->as<ob::PointsFrame>()->getCoordinateValueScale() * scale;
		const uint64_t tDNs = frameTsNs(spFrame);

		points->frameStart();

		if (format == OB_FORMAT_RGB_POINT)
		{
			const size_t nP = spFrame->getDataSize() / sizeof(OBColorPoint);
			const auto *pts = reinterpret_cast<const OBColorPoint *>(spFrame->getData());
			constexpr float c_b = 1.0f / 255.0f;

			for (size_t i = 0; i < nP; ++i)
			{
				const auto &p = pts[i];
				IF_CONT(!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z));
				IF_CONT(p.z <= 0);

				const Vector3f vP(p.x * s_b, p.y * s_b, p.z * s_b);
				// The filter preserves the configured BGR stream's channel order.
				const Vector3f vC(p.b * c_b, p.g * c_b, p.r * c_b);
				points->add(vP, vC, tDNs);
			}
		}
		else
		{
			const size_t nP = spFrame->getDataSize() / sizeof(OBPoint);
			const auto *pts = reinterpret_cast<const OBPoint *>(spFrame->getData());
			const Vector3f vC(1, 1, 1);

			for (size_t i = 0; i < nP; ++i)
			{
				const auto &p = pts[i];
				IF_CONT(!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z));
				IF_CONT(p.z <= 0);

				const Vector3f vP(p.x * s_b, p.y * s_b, p.z * s_b);
				points->add(vP, vC, tDNs);
			}
		}

		points->frameStop();
#endif
	}

	void _Orbbec::console(void *pConsole)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		NULL_(pConsole);
		this->_RGBDbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("tDNs = " + li2str(m_tDNs) + ", dtDNs = " + li2str(m_dtDNs));
		pC->addMsg("tRGBNs = " + li2str(m_tRGBNs) + ", dtRGBNs = " + li2str(m_dtRGBNs));
	}

	void _Orbbec::console(const json &j, void *pJSONbase)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		auto *pJb = static_cast<_JSONbase *>(pJSONbase);
		if (!pJb || !j.is_object() || !j.contains("cmd") || !j["cmd"].is_string()) return;
		const string cmd = j["cmd"].get<string>();
		if (cmd != "getConfig" && cmd != "setConfig" && cmd != "saveConfig") return;
		json reply = {{"cmd", cmd}, {"module", getName()}, {"bSuccess", true}};
		if (j.contains("requestId")) reply["requestId"] = j["requestId"];
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
				reply["bSuccess"] = saveConfig();
				if (!reply["bSuccess"].get<bool>())
				{
					reply["error"] = "Could not save the launch configuration";
				}
			}
			// Return the current settings and schema for the control panel.
			reply["config"] = configValues();
			if (cmd == "getConfig") reply["schema"] = controlSchema();
			reply["deviceOpen"] = m_bOpened;
		}
		catch (const std::exception &e) { reply["bSuccess"] = false; reply["error"] = e.what(); }
		pJb->sendJson(reply);
	}

	OrbbecCtrl _Orbbec::getCamCtrl(void) const
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		return m_orbbecCtrl;
	}

	bool _Orbbec::setCamCtrl(const OrbbecCtrl &camCtrl)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl = camCtrl;
		NULL_F(m_spDev);
		bool success = true;

		// Presets and work modes can reset properties, so load them first.
		if (m_orbbecCtrl.m_depthWorkMode)
			success = switchDepthWorkMode(*m_orbbecCtrl.m_depthWorkMode) && success;
		if (m_orbbecCtrl.m_preset)
			success = loadPreset(*m_orbbecCtrl.m_preset) && success;
		if (m_orbbecCtrl.m_presetJsonFile)
			success = loadPresetFromJsonFile(*m_orbbecCtrl.m_presetJsonFile) && success;
		if (m_orbbecCtrl.m_colorPreset)
			success = switchColorPreset(*m_orbbecCtrl.m_colorPreset) && success;
		if (m_orbbecCtrl.m_frameInterleave)
			success = loadFrameInterleave(*m_orbbecCtrl.m_frameInterleave) && success;

		// Composite synchronization is applied before individual timing overrides.
		if (m_orbbecCtrl.m_multiDeviceSyncConfig)
			success = setMultiDeviceSyncConfig(*m_orbbecCtrl.m_multiDeviceSyncConfig) && success;
		if (m_orbbecCtrl.m_timestampResetConfig)
			success = setTimestampResetConfig(*m_orbbecCtrl.m_timestampResetConfig) && success;

		// Disable HDR before restoring automatic exposure; enable it after exposure controls.
		if (m_orbbecCtrl.m_depthHdrConfig && !m_orbbecCtrl.m_depthHdrConfig->enable)
			success = setDepthHdrConfig(*m_orbbecCtrl.m_depthHdrConfig) && success;
		if (m_orbbecCtrl.m_ldp)
			success = setLDP(*m_orbbecCtrl.m_ldp) && success;
		if (m_orbbecCtrl.m_laser)
			success = setLaser(*m_orbbecCtrl.m_laser) && success;
		if (m_orbbecCtrl.m_flood)
			success = setFlood(*m_orbbecCtrl.m_flood) && success;
		if (m_orbbecCtrl.m_temperatureCompensation)
			success = setTemperatureCompensation(*m_orbbecCtrl.m_temperatureCompensation) && success;
		if (m_orbbecCtrl.m_depthMirror)
			success = setDepthMirror(*m_orbbecCtrl.m_depthMirror) && success;
		if (m_orbbecCtrl.m_depthFlip)
			success = setDepthFlip(*m_orbbecCtrl.m_depthFlip) && success;
		if (m_orbbecCtrl.m_depthHolefilter)
			success = setDepthHolefilter(*m_orbbecCtrl.m_depthHolefilter) && success;
		if (m_orbbecCtrl.m_irMirror)
			success = setIRMirror(*m_orbbecCtrl.m_irMirror) && success;
		if (m_orbbecCtrl.m_irFlip)
			success = setIRFlip(*m_orbbecCtrl.m_irFlip) && success;
		if (m_orbbecCtrl.m_depthNoiseRemovalFilter)
			success = setDepthNoiseRemovalFilter(*m_orbbecCtrl.m_depthNoiseRemovalFilter) && success;
		if (m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff)
			success = setDepthNoiseRemovalFilterMaxDiff(*m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff) && success;
		if (m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize)
			success = setDepthNoiseRemovalFilterMaxSpeckleSize(*m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize) && success;
		if (m_orbbecCtrl.m_depthAlignHardware)
			success = setDepthAlignHardware(*m_orbbecCtrl.m_depthAlignHardware) && success;
		if (m_orbbecCtrl.m_depthAlignHardwareMode)
			success = setDepthAlignHardwareMode(*m_orbbecCtrl.m_depthAlignHardwareMode) && success;
		if (m_orbbecCtrl.m_depthPrecisionLevel)
			success = setDepthPrecisionLevel(*m_orbbecCtrl.m_depthPrecisionLevel) && success;
		if (m_orbbecCtrl.m_colorMirror)
			success = setColorMirror(*m_orbbecCtrl.m_colorMirror) && success;
		if (m_orbbecCtrl.m_colorFlip)
			success = setColorFlip(*m_orbbecCtrl.m_colorFlip) && success;
		if (m_orbbecCtrl.m_disparityToDepth)
			success = setDisparityToDepth(*m_orbbecCtrl.m_disparityToDepth) && success;
		if (m_orbbecCtrl.m_watchdog)
			success = setWatchdog(*m_orbbecCtrl.m_watchdog) && success;
		if (m_orbbecCtrl.m_externalSignalReset)
			success = setExternalSignalReset(*m_orbbecCtrl.m_externalSignalReset) && success;
		if (m_orbbecCtrl.m_heartbeat)
			success = setHeartbeat(*m_orbbecCtrl.m_heartbeat) && success;
		if (m_orbbecCtrl.m_laserPowerLevelControl)
			success = setLaserPowerLevelControl(*m_orbbecCtrl.m_laserPowerLevelControl) && success;
		if (m_orbbecCtrl.m_timerResetTriggerOutEnable)
			success = setTimerResetTriggerOutEnable(*m_orbbecCtrl.m_timerResetTriggerOutEnable) && success;
		if (m_orbbecCtrl.m_timerResetDelayUs)
			success = setTimerResetDelayUs(*m_orbbecCtrl.m_timerResetDelayUs) && success;
		if (m_orbbecCtrl.m_irRightMirror)
			success = setIRRightMirror(*m_orbbecCtrl.m_irRightMirror) && success;
		if (m_orbbecCtrl.m_captureImageFrameNumber)
			success = setCaptureImageFrameNumber(*m_orbbecCtrl.m_captureImageFrameNumber) && success;
		if (m_orbbecCtrl.m_irRightFlip)
			success = setIRRightFlip(*m_orbbecCtrl.m_irRightFlip) && success;
		if (m_orbbecCtrl.m_colorRotate)
			success = setColorRotate(*m_orbbecCtrl.m_colorRotate) && success;
		if (m_orbbecCtrl.m_irRotate)
			success = setIRRotate(*m_orbbecCtrl.m_irRotate) && success;
		if (m_orbbecCtrl.m_irRightRotate)
			success = setIRRightRotate(*m_orbbecCtrl.m_irRightRotate) && success;
		if (m_orbbecCtrl.m_depthRotate)
			success = setDepthRotate(*m_orbbecCtrl.m_depthRotate) && success;
		if (m_orbbecCtrl.m_syncSignalTriggerOut)
			success = setSyncSignalTriggerOut(*m_orbbecCtrl.m_syncSignalTriggerOut) && success;
		if (m_orbbecCtrl.m_deviceUSB2RepeatIdentify)
			success = setDeviceUSB2RepeatIdentify(*m_orbbecCtrl.m_deviceUSB2RepeatIdentify) && success;
		if (m_orbbecCtrl.m_laserAlwaysOn)
			success = setLaserAlwaysOn(*m_orbbecCtrl.m_laserAlwaysOn) && success;
		if (m_orbbecCtrl.m_laserOnOffPattern)
			success = setLaserOnOffPattern(*m_orbbecCtrl.m_laserOnOffPattern) && success;
		if (m_orbbecCtrl.m_depthUnitFlexibleAdjustment)
			success = setDepthUnitFlexibleAdjustment(*m_orbbecCtrl.m_depthUnitFlexibleAdjustment) && success;
		if (m_orbbecCtrl.m_laserControl)
			success = setLaserControl(*m_orbbecCtrl.m_laserControl) && success;
		if (m_orbbecCtrl.m_irBrightness)
			success = setIRBrightness(*m_orbbecCtrl.m_irBrightness) && success;
		if (m_orbbecCtrl.m_colorAEMaxExposure)
			success = setColorAEMaxExposure(*m_orbbecCtrl.m_colorAEMaxExposure) && success;
		if (m_orbbecCtrl.m_irAEMaxExposure)
			success = setIRAEMaxExposure(*m_orbbecCtrl.m_irAEMaxExposure) && success;
		if (m_orbbecCtrl.m_dispSearchRangeMode)
			success = setDispSearchRangeMode(*m_orbbecCtrl.m_dispSearchRangeMode) && success;
		if (m_orbbecCtrl.m_dispSearchOffset)
			success = setDispSearchOffset(*m_orbbecCtrl.m_dispSearchOffset) && success;
		if (m_orbbecCtrl.m_cpuTemperatureCalibration)
			success = setCPUTemperatureCalibration(*m_orbbecCtrl.m_cpuTemperatureCalibration) && success;
		if (m_orbbecCtrl.m_frameInterleaveConfigIndex)
			success = setFrameInterleaveConfigIndex(*m_orbbecCtrl.m_frameInterleaveConfigIndex) && success;
		if (m_orbbecCtrl.m_frameInterleaveEnable)
			success = setFrameInterleaveEnable(*m_orbbecCtrl.m_frameInterleaveEnable) && success;
		if (m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay)
			success = setFrameInterleaveLaserPatternSyncDelay(*m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay) && success;
		if (m_orbbecCtrl.m_onChipCalibrationEnable)
			success = setOnChipCalibrationEnable(*m_orbbecCtrl.m_onChipCalibrationEnable) && success;
		if (m_orbbecCtrl.m_hwNoiseRemoveFilterEnable)
			success = setHWNoiseRemoveFilterEnable(*m_orbbecCtrl.m_hwNoiseRemoveFilterEnable) && success;
		if (m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold)
			success = setHWNoiseRemoveFilterThreshold(*m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold) && success;
		if (m_orbbecCtrl.m_deviceAutoCaptureEnable)
			success = setDeviceAutoCaptureEnable(*m_orbbecCtrl.m_deviceAutoCaptureEnable) && success;
		if (m_orbbecCtrl.m_deviceAutoCaptureIntervalTime)
			success = setDeviceAutoCaptureIntervalTime(*m_orbbecCtrl.m_deviceAutoCaptureIntervalTime) && success;
		if (m_orbbecCtrl.m_devicePTPClockSyncEnable)
			success = setDevicePTPClockSyncEnable(*m_orbbecCtrl.m_devicePTPClockSyncEnable) && success;
		if (m_orbbecCtrl.m_confidenceStreamFilter)
			success = setConfidenceStreamFilter(*m_orbbecCtrl.m_confidenceStreamFilter) && success;
		if (m_orbbecCtrl.m_confidenceStreamFilterThreshold)
			success = setConfidenceStreamFilterThreshold(*m_orbbecCtrl.m_confidenceStreamFilterThreshold) && success;
		if (m_orbbecCtrl.m_confidenceMirror)
			success = setConfidenceMirror(*m_orbbecCtrl.m_confidenceMirror) && success;
		if (m_orbbecCtrl.m_confidenceFlip)
			success = setConfidenceFlip(*m_orbbecCtrl.m_confidenceFlip) && success;
		if (m_orbbecCtrl.m_confidenceRotate)
			success = setConfidenceRotate(*m_orbbecCtrl.m_confidenceRotate) && success;
		if (m_orbbecCtrl.m_intraCameraSyncReference)
			success = setIntraCameraSyncReference(*m_orbbecCtrl.m_intraCameraSyncReference) && success;
		if (m_orbbecCtrl.m_colorRightRotate)
			success = setColorRightRotate(*m_orbbecCtrl.m_colorRightRotate) && success;
		if (m_orbbecCtrl.m_colorRightMirror)
			success = setColorRightMirror(*m_orbbecCtrl.m_colorRightMirror) && success;
		if (m_orbbecCtrl.m_colorRightFlip)
			success = setColorRightFlip(*m_orbbecCtrl.m_colorRightFlip) && success;
		if (m_orbbecCtrl.m_deviceAEReference)
			success = setDeviceAEReference(*m_orbbecCtrl.m_deviceAEReference) && success;
		if (m_orbbecCtrl.m_deviceAEStrategy)
			success = setDeviceAEStrategy(*m_orbbecCtrl.m_deviceAEStrategy) && success;
		if (m_orbbecCtrl.m_colorROIBrightness)
			success = setColorROIBrightness(*m_orbbecCtrl.m_colorROIBrightness) && success;
		if (m_orbbecCtrl.m_colorLeftRotate)
			success = setColorLeftRotate(*m_orbbecCtrl.m_colorLeftRotate) && success;
		if (m_orbbecCtrl.m_colorLeftMirror)
			success = setColorLeftMirror(*m_orbbecCtrl.m_colorLeftMirror) && success;
		if (m_orbbecCtrl.m_colorLeftFlip)
			success = setColorLeftFlip(*m_orbbecCtrl.m_colorLeftFlip) && success;
		if (m_orbbecCtrl.m_colorPresetPriority)
			success = setColorPresetPriority(*m_orbbecCtrl.m_colorPresetPriority) && success;
		if (m_orbbecCtrl.m_deviceNetworkLLA)
			success = setDeviceNetworkLLA(*m_orbbecCtrl.m_deviceNetworkLLA) && success;
		if (m_orbbecCtrl.m_colorAntiFlicker)
			success = setColorAntiFlicker(*m_orbbecCtrl.m_colorAntiFlicker) && success;
		if (m_orbbecCtrl.m_deviceIPMode)
			success = setDeviceIPMode(*m_orbbecCtrl.m_deviceIPMode) && success;
		if (m_orbbecCtrl.m_dhcpAssignIPTimeout)
			success = setDHCPAssignIPTimeout(*m_orbbecCtrl.m_dhcpAssignIPTimeout) && success;
		if (m_orbbecCtrl.m_usbSyncVoltageLevel)
			success = setUSBSyncVoltageLevel(*m_orbbecCtrl.m_usbSyncVoltageLevel) && success;
		if (m_orbbecCtrl.m_fpsBoost)
			success = setFPSBoost(*m_orbbecCtrl.m_fpsBoost) && success;
		if (m_orbbecCtrl.m_mjpegQuality)
			success = setMJPEGQuality(*m_orbbecCtrl.m_mjpegQuality) && success;
		if (m_orbbecCtrl.m_colorAutoExposure)
			success = setColorAutoExposure(*m_orbbecCtrl.m_colorAutoExposure) && success;
		if (m_orbbecCtrl.m_colorExposure)
			success = setColorExposure(*m_orbbecCtrl.m_colorExposure) && success;
		if (m_orbbecCtrl.m_colorGain)
			success = setColorGain(*m_orbbecCtrl.m_colorGain) && success;
		if (m_orbbecCtrl.m_colorAutoWhiteBalance)
			success = setColorAutoWhiteBalance(*m_orbbecCtrl.m_colorAutoWhiteBalance) && success;
		if (m_orbbecCtrl.m_colorWhiteBalance)
			success = setColorWhiteBalance(*m_orbbecCtrl.m_colorWhiteBalance) && success;
		if (m_orbbecCtrl.m_colorBrightness)
			success = setColorBrightness(*m_orbbecCtrl.m_colorBrightness) && success;
		if (m_orbbecCtrl.m_colorSharpness)
			success = setColorSharpness(*m_orbbecCtrl.m_colorSharpness) && success;
		if (m_orbbecCtrl.m_colorSaturation)
			success = setColorSaturation(*m_orbbecCtrl.m_colorSaturation) && success;
		if (m_orbbecCtrl.m_colorContrast)
			success = setColorContrast(*m_orbbecCtrl.m_colorContrast) && success;
		if (m_orbbecCtrl.m_colorGamma)
			success = setColorGamma(*m_orbbecCtrl.m_colorGamma) && success;
		if (m_orbbecCtrl.m_colorAutoExposurePriority)
			success = setColorAutoExposurePriority(*m_orbbecCtrl.m_colorAutoExposurePriority) && success;
		if (m_orbbecCtrl.m_colorBacklightCompensation)
			success = setColorBacklightCompensation(*m_orbbecCtrl.m_colorBacklightCompensation) && success;
		if (m_orbbecCtrl.m_colorHue)
			success = setColorHue(*m_orbbecCtrl.m_colorHue) && success;
		if (m_orbbecCtrl.m_colorPowerLineFrequency)
			success = setColorPowerLineFrequency(*m_orbbecCtrl.m_colorPowerLineFrequency) && success;
		if (m_orbbecCtrl.m_depthAutoExposure)
			success = setDepthAutoExposure(*m_orbbecCtrl.m_depthAutoExposure) && success;
		if (m_orbbecCtrl.m_depthExposure)
			success = setDepthExposure(*m_orbbecCtrl.m_depthExposure) && success;
		if (m_orbbecCtrl.m_depthGain)
			success = setDepthGain(*m_orbbecCtrl.m_depthGain) && success;
		if (m_orbbecCtrl.m_irAutoExposure)
			success = setIRAutoExposure(*m_orbbecCtrl.m_irAutoExposure) && success;
		if (m_orbbecCtrl.m_irExposure)
			success = setIRExposure(*m_orbbecCtrl.m_irExposure) && success;
		if (m_orbbecCtrl.m_irGain)
			success = setIRGain(*m_orbbecCtrl.m_irGain) && success;
		if (m_orbbecCtrl.m_irChannelDataSource)
			success = setIRChannelDataSource(*m_orbbecCtrl.m_irChannelDataSource) && success;
		if (m_orbbecCtrl.m_depthRMFilter)
			success = setDepthRMFilter(*m_orbbecCtrl.m_depthRMFilter) && success;
		if (m_orbbecCtrl.m_colorAEMaxGain)
			success = setColorAEMaxGain(*m_orbbecCtrl.m_colorAEMaxGain) && success;
		if (m_orbbecCtrl.m_depthAutoExposurePriority)
			success = setDepthAutoExposurePriority(*m_orbbecCtrl.m_depthAutoExposurePriority) && success;
		if (m_orbbecCtrl.m_sdkDisparityToDepth)
			success = setSDKDisparityToDepth(*m_orbbecCtrl.m_sdkDisparityToDepth) && success;
		if (m_orbbecCtrl.m_sdkAccelFrameTransformed)
			success = setSDKAccelFrameTransformed(*m_orbbecCtrl.m_sdkAccelFrameTransformed) && success;
		if (m_orbbecCtrl.m_sdkGyroFrameTransformed)
			success = setSDKGyroFrameTransformed(*m_orbbecCtrl.m_sdkGyroFrameTransformed) && success;
		if (m_orbbecCtrl.m_devicePerformanceMode)
			success = setDevicePerformanceMode(*m_orbbecCtrl.m_devicePerformanceMode) && success;
		if (m_orbbecCtrl.m_colorDenoisingLevel)
			success = setColorDenoisingLevel(*m_orbbecCtrl.m_colorDenoisingLevel) && success;

		if (m_orbbecCtrl.m_deviceIPAddrConfig)
			success = setDeviceIPAddrConfig(*m_orbbecCtrl.m_deviceIPAddrConfig) && success;
		if (m_orbbecCtrl.m_colorAEROI)
			success = setColorAEROI(*m_orbbecCtrl.m_colorAEROI) && success;
		if (m_orbbecCtrl.m_depthAEROI)
			success = setDepthAEROI(*m_orbbecCtrl.m_depthAEROI) && success;
		if (m_orbbecCtrl.m_dispOffsetConfig)
			success = setDispOffsetConfig(*m_orbbecCtrl.m_dispOffsetConfig) && success;
		if (m_orbbecCtrl.m_presetResolutionConfig)
			success = setPresetResolutionConfig(*m_orbbecCtrl.m_presetResolutionConfig) && success;
		if (m_orbbecCtrl.m_deviceIPAddrConfigV2)
			success = setDeviceIPAddrConfigV2(*m_orbbecCtrl.m_deviceIPAddrConfigV2) && success;
		if (m_orbbecCtrl.m_depthHdrConfig && m_orbbecCtrl.m_depthHdrConfig->enable)
			success = setDepthHdrConfig(*m_orbbecCtrl.m_depthHdrConfig) && success;
		if (m_orbbecCtrl.m_globalTimestamp)
			success = enableGlobalTimestamp(*m_orbbecCtrl.m_globalTimestamp) && success;
		if (m_orbbecCtrl.m_firmwareLog)
			success = enableFirmwareLog(*m_orbbecCtrl.m_firmwareLog) && success;

		return success;
	}

	bool _Orbbec::setBoolProperty(OBPropertyID propertyId, bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		NULL_F(m_spDev);
		try
		{
			if (!m_spDev->isPropertySupported(propertyId, OB_PERMISSION_WRITE))
			{
				LOG_I("Orbbec property is not writable on this device: " + i2str(propertyId));
				return false;
			}
			m_spDev->setBoolProperty(propertyId, value);
		}
		catch (const ob::Error &e)
		{
			LOG_E("Cannot set Orbbec property " + i2str(propertyId) + ": " + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::setIntProperty(OBPropertyID propertyId, int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		NULL_F(m_spDev);
		try
		{
			if (!m_spDev->isPropertySupported(propertyId, OB_PERMISSION_WRITE))
			{
				LOG_I("Orbbec property is not writable on this device: " + i2str(propertyId));
				return false;
			}
			m_spDev->setIntProperty(propertyId, value);
		}
		catch (const ob::Error &e)
		{
			LOG_E("Cannot set Orbbec property " + i2str(propertyId) + ": " + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::setFloatProperty(OBPropertyID propertyId, float value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		NULL_F(m_spDev);
		try
		{
			if (!m_spDev->isPropertySupported(propertyId, OB_PERMISSION_WRITE))
			{
				LOG_I("Orbbec property is not writable on this device: " + i2str(propertyId));
				return false;
			}
			IF_F(!std::isfinite(value));
			m_spDev->setFloatProperty(propertyId, value);
		}
		catch (const ob::Error &e)
		{
			LOG_E("Cannot set Orbbec property " + i2str(propertyId) + ": " + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::setStructuredData(OBPropertyID propertyId, const void *data, uint32_t size)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		NULL_F(m_spDev);
		try
		{
			if (!m_spDev->isPropertySupported(propertyId, OB_PERMISSION_WRITE))
			{
				LOG_I("Orbbec property is not writable on this device: " + i2str(propertyId));
				return false;
			}
			m_spDev->setStructuredData(propertyId, static_cast<const uint8_t *>(data), size);
		}
		catch (const ob::Error &e)
		{
			LOG_E("Cannot set Orbbec property " + i2str(propertyId) + ": " + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::setLDP(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_ldp = value;
		return setBoolProperty(OB_PROP_LDP_BOOL, value);
	}

	bool _Orbbec::setLaser(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_laser = value;
		return setBoolProperty(OB_PROP_LASER_BOOL, value);
	}

	bool _Orbbec::setFlood(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_flood = value;
		return setBoolProperty(OB_PROP_FLOOD_BOOL, value);
	}

	bool _Orbbec::setTemperatureCompensation(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_temperatureCompensation = value;
		return setBoolProperty(OB_PROP_TEMPERATURE_COMPENSATION_BOOL, value);
	}

	bool _Orbbec::setDepthMirror(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthMirror = value;
		return setBoolProperty(OB_PROP_DEPTH_MIRROR_BOOL, value);
	}

	bool _Orbbec::setDepthFlip(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthFlip = value;
		return setBoolProperty(OB_PROP_DEPTH_FLIP_BOOL, value);
	}

	bool _Orbbec::setDepthHolefilter(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthHolefilter = value;
		return setBoolProperty(OB_PROP_DEPTH_HOLEFILTER_BOOL, value);
	}

	bool _Orbbec::setIRMirror(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irMirror = value;
		return setBoolProperty(OB_PROP_IR_MIRROR_BOOL, value);
	}

	bool _Orbbec::setIRFlip(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irFlip = value;
		return setBoolProperty(OB_PROP_IR_FLIP_BOOL, value);
	}

	bool _Orbbec::setDepthNoiseRemovalFilter(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthNoiseRemovalFilter = value;
		return setBoolProperty(OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL, value);
	}

	bool _Orbbec::setDepthNoiseRemovalFilterMaxDiff(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff = value;
		return setIntProperty(OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT, value);
	}

	bool _Orbbec::setDepthNoiseRemovalFilterMaxSpeckleSize(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize = value;
		return setIntProperty(OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT, value);
	}

	bool _Orbbec::setDepthAlignHardware(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthAlignHardware = value;
		return setBoolProperty(OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL, value);
	}

	bool _Orbbec::setDepthAlignHardwareMode(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthAlignHardwareMode = value;
		return setIntProperty(OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT, value);
	}

	bool _Orbbec::setDepthPrecisionLevel(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthPrecisionLevel = value;
		return setIntProperty(OB_PROP_DEPTH_PRECISION_LEVEL_INT, value);
	}

	bool _Orbbec::setColorMirror(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorMirror = value;
		return setBoolProperty(OB_PROP_COLOR_MIRROR_BOOL, value);
	}

	bool _Orbbec::setColorFlip(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorFlip = value;
		return setBoolProperty(OB_PROP_COLOR_FLIP_BOOL, value);
	}

	bool _Orbbec::setDisparityToDepth(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_disparityToDepth = value;
		return setBoolProperty(OB_PROP_DISPARITY_TO_DEPTH_BOOL, value);
	}

	bool _Orbbec::setWatchdog(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_watchdog = value;
		return setBoolProperty(OB_PROP_WATCHDOG_BOOL, value);
	}

	bool _Orbbec::setExternalSignalReset(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_externalSignalReset = value;
		return setBoolProperty(OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL, value);
	}

	bool _Orbbec::setHeartbeat(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_heartbeat = value;
		return setBoolProperty(OB_PROP_HEARTBEAT_BOOL, value);
	}

	bool _Orbbec::setLaserPowerLevelControl(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_laserPowerLevelControl = value;
		return setIntProperty(OB_PROP_LASER_POWER_LEVEL_CONTROL_INT, value);
	}

	bool _Orbbec::setTimerResetTriggerOutEnable(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_timerResetTriggerOutEnable = value;
		return setBoolProperty(OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL, value);
	}

	bool _Orbbec::setTimerResetDelayUs(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_timerResetDelayUs = value;
		return setIntProperty(OB_PROP_TIMER_RESET_DELAY_US_INT, value);
	}

	bool _Orbbec::setIRRightMirror(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irRightMirror = value;
		return setBoolProperty(OB_PROP_IR_RIGHT_MIRROR_BOOL, value);
	}

	bool _Orbbec::setCaptureImageFrameNumber(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_captureImageFrameNumber = value;
		return setIntProperty(OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT, value);
	}

	bool _Orbbec::setIRRightFlip(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irRightFlip = value;
		return setBoolProperty(OB_PROP_IR_RIGHT_FLIP_BOOL, value);
	}

	bool _Orbbec::setColorRotate(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorRotate = value;
		return setIntProperty(OB_PROP_COLOR_ROTATE_INT, value);
	}

	bool _Orbbec::setIRRotate(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irRotate = value;
		return setIntProperty(OB_PROP_IR_ROTATE_INT, value);
	}

	bool _Orbbec::setIRRightRotate(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irRightRotate = value;
		return setIntProperty(OB_PROP_IR_RIGHT_ROTATE_INT, value);
	}

	bool _Orbbec::setDepthRotate(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthRotate = value;
		return setIntProperty(OB_PROP_DEPTH_ROTATE_INT, value);
	}

	bool _Orbbec::setSyncSignalTriggerOut(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_syncSignalTriggerOut = value;
		return setBoolProperty(OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL, value);
	}

	bool _Orbbec::setDeviceUSB2RepeatIdentify(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceUSB2RepeatIdentify = value;
		return setBoolProperty(OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL, value);
	}

	bool _Orbbec::setLaserAlwaysOn(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_laserAlwaysOn = value;
		return setBoolProperty(OB_PROP_LASER_ALWAYS_ON_BOOL, value);
	}

	bool _Orbbec::setLaserOnOffPattern(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_laserOnOffPattern = value;
		return setIntProperty(OB_PROP_LASER_ON_OFF_PATTERN_INT, value);
	}

	bool _Orbbec::setDepthUnitFlexibleAdjustment(float value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthUnitFlexibleAdjustment = value;
		return setFloatProperty(OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT, value);
	}

	bool _Orbbec::setLaserControl(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_laserControl = value;
		return setIntProperty(OB_PROP_LASER_CONTROL_INT, value);
	}

	bool _Orbbec::setIRBrightness(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irBrightness = value;
		return setIntProperty(OB_PROP_IR_BRIGHTNESS_INT, value);
	}

	bool _Orbbec::setColorAEMaxExposure(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorAEMaxExposure = value;
		return setIntProperty(OB_PROP_COLOR_AE_MAX_EXPOSURE_INT, value);
	}

	bool _Orbbec::setIRAEMaxExposure(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irAEMaxExposure = value;
		return setIntProperty(OB_PROP_IR_AE_MAX_EXPOSURE_INT, value);
	}

	bool _Orbbec::setDispSearchRangeMode(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_dispSearchRangeMode = value;
		return setIntProperty(OB_PROP_DISP_SEARCH_RANGE_MODE_INT, value);
	}

	bool _Orbbec::setDispSearchOffset(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_dispSearchOffset = value;
		return setIntProperty(OB_PROP_DISP_SEARCH_OFFSET_INT, value);
	}

	bool _Orbbec::setCPUTemperatureCalibration(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_cpuTemperatureCalibration = value;
		return setBoolProperty(OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL, value);
	}

	bool _Orbbec::setFrameInterleaveConfigIndex(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_frameInterleaveConfigIndex = value;
		return setIntProperty(OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT, value);
	}

	bool _Orbbec::setFrameInterleaveEnable(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_frameInterleaveEnable = value;
		return setBoolProperty(OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL, value);
	}

	bool _Orbbec::setFrameInterleaveLaserPatternSyncDelay(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay = value;
		return setIntProperty(OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT, value);
	}

	bool _Orbbec::setOnChipCalibrationEnable(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_onChipCalibrationEnable = value;
		return setBoolProperty(OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL, value);
	}

	bool _Orbbec::setHWNoiseRemoveFilterEnable(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_hwNoiseRemoveFilterEnable = value;
		return setBoolProperty(OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL, value);
	}

	bool _Orbbec::setHWNoiseRemoveFilterThreshold(float value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold = value;
		return setFloatProperty(OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT, value);
	}

	bool _Orbbec::setDeviceAutoCaptureEnable(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceAutoCaptureEnable = value;
		return setBoolProperty(OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL, value);
	}

	bool _Orbbec::setDeviceAutoCaptureIntervalTime(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceAutoCaptureIntervalTime = value;
		return setIntProperty(OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT, value);
	}

	bool _Orbbec::setDevicePTPClockSyncEnable(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_devicePTPClockSyncEnable = value;
		return setBoolProperty(OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL, value);
	}

	bool _Orbbec::setConfidenceStreamFilter(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_confidenceStreamFilter = value;
		return setBoolProperty(OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL, value);
	}

	bool _Orbbec::setConfidenceStreamFilterThreshold(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_confidenceStreamFilterThreshold = value;
		return setIntProperty(OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT, value);
	}

	bool _Orbbec::setConfidenceMirror(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_confidenceMirror = value;
		return setBoolProperty(OB_PROP_CONFIDENCE_MIRROR_BOOL, value);
	}

	bool _Orbbec::setConfidenceFlip(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_confidenceFlip = value;
		return setBoolProperty(OB_PROP_CONFIDENCE_FLIP_BOOL, value);
	}

	bool _Orbbec::setConfidenceRotate(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_confidenceRotate = value;
		return setIntProperty(OB_PROP_CONFIDENCE_ROTATE_INT, value);
	}

	bool _Orbbec::setIntraCameraSyncReference(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_intraCameraSyncReference = value;
		return setIntProperty(OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT, value);
	}

	bool _Orbbec::setColorRightRotate(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorRightRotate = value;
		return setIntProperty(OB_PROP_COLOR_RIGHT_ROTATE_INT, value);
	}

	bool _Orbbec::setColorRightMirror(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorRightMirror = value;
		return setBoolProperty(OB_PROP_COLOR_RIGHT_MIRROR_BOOL, value);
	}

	bool _Orbbec::setColorRightFlip(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorRightFlip = value;
		return setBoolProperty(OB_PROP_COLOR_RIGHT_FLIP_BOOL, value);
	}

	bool _Orbbec::setDeviceAEReference(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceAEReference = value;
		return setIntProperty(OB_PROP_DEVICE_AE_REFERENCE_INT, value);
	}

	bool _Orbbec::setDeviceAEStrategy(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceAEStrategy = value;
		return setIntProperty(OB_PROP_DEVICE_AE_STRATEGY_INT, value);
	}

	bool _Orbbec::setColorROIBrightness(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorROIBrightness = value;
		return setIntProperty(OB_PROP_COLOR_ROI_BRIGHTNESS_INT, value);
	}

	bool _Orbbec::setColorLeftRotate(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorLeftRotate = value;
		return setIntProperty(OB_PROP_COLOR_LEFT_ROTATE_INT, value);
	}

	bool _Orbbec::setColorLeftMirror(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorLeftMirror = value;
		return setBoolProperty(OB_PROP_COLOR_LEFT_MIRROR_BOOL, value);
	}

	bool _Orbbec::setColorLeftFlip(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorLeftFlip = value;
		return setBoolProperty(OB_PROP_COLOR_LEFT_FLIP_BOOL, value);
	}

	bool _Orbbec::setColorPresetPriority(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorPresetPriority = value;
		return setIntProperty(OB_PROP_COLOR_PRESET_PRIORITY_INT, value);
	}

	bool _Orbbec::setDeviceNetworkLLA(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceNetworkLLA = value;
		return setBoolProperty(OB_PROP_DEVICE_NETWORK_LLA_BOOL, value);
	}

	bool _Orbbec::setColorAntiFlicker(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorAntiFlicker = value;
		return setBoolProperty(OB_PROP_COLOR_ANTI_FLICKER_BOOL, value);
	}

	bool _Orbbec::setDeviceIPMode(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceIPMode = value;
		return setIntProperty(OB_PROP_DEVICE_IP_MODE_INT, value);
	}

	bool _Orbbec::setDHCPAssignIPTimeout(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_dhcpAssignIPTimeout = value;
		return setIntProperty(OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT, value);
	}

	bool _Orbbec::setUSBSyncVoltageLevel(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_usbSyncVoltageLevel = value;
		return setIntProperty(OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT, value);
	}

	bool _Orbbec::setFPSBoost(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_fpsBoost = value;
		return setBoolProperty(OB_PROP_FPS_BOOST_BOOL, value);
	}

	bool _Orbbec::setMJPEGQuality(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_mjpegQuality = value;
		return setIntProperty(OB_PROP_MJPEG_QUALITY_INT, value);
	}

	bool _Orbbec::setColorAutoExposure(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorAutoExposure = value;
		return setBoolProperty(OB_PROP_COLOR_AUTO_EXPOSURE_BOOL, value);
	}

	bool _Orbbec::setColorExposure(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorExposure = value;
		return setIntProperty(OB_PROP_COLOR_EXPOSURE_INT, value);
	}

	bool _Orbbec::setColorGain(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorGain = value;
		return setIntProperty(OB_PROP_COLOR_GAIN_INT, value);
	}

	bool _Orbbec::setColorAutoWhiteBalance(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorAutoWhiteBalance = value;
		return setBoolProperty(OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL, value);
	}

	bool _Orbbec::setColorWhiteBalance(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorWhiteBalance = value;
		return setIntProperty(OB_PROP_COLOR_WHITE_BALANCE_INT, value);
	}

	bool _Orbbec::setColorBrightness(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorBrightness = value;
		return setIntProperty(OB_PROP_COLOR_BRIGHTNESS_INT, value);
	}

	bool _Orbbec::setColorSharpness(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorSharpness = value;
		return setIntProperty(OB_PROP_COLOR_SHARPNESS_INT, value);
	}

	bool _Orbbec::setColorSaturation(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorSaturation = value;
		return setIntProperty(OB_PROP_COLOR_SATURATION_INT, value);
	}

	bool _Orbbec::setColorContrast(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorContrast = value;
		return setIntProperty(OB_PROP_COLOR_CONTRAST_INT, value);
	}

	bool _Orbbec::setColorGamma(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorGamma = value;
		return setIntProperty(OB_PROP_COLOR_GAMMA_INT, value);
	}

	bool _Orbbec::setColorAutoExposurePriority(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorAutoExposurePriority = value;
		return setIntProperty(OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT, value);
	}

	bool _Orbbec::setColorBacklightCompensation(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorBacklightCompensation = value;
		return setIntProperty(OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT, value);
	}

	bool _Orbbec::setColorHue(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorHue = value;
		return setIntProperty(OB_PROP_COLOR_HUE_INT, value);
	}

	bool _Orbbec::setColorPowerLineFrequency(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorPowerLineFrequency = value;
		return setIntProperty(OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT, value);
	}

	bool _Orbbec::setDepthAutoExposure(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthAutoExposure = value;
		return setBoolProperty(OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL, value);
	}

	bool _Orbbec::setDepthExposure(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthExposure = value;
		return setIntProperty(OB_PROP_DEPTH_EXPOSURE_INT, value);
	}

	bool _Orbbec::setDepthGain(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthGain = value;
		return setIntProperty(OB_PROP_DEPTH_GAIN_INT, value);
	}

	bool _Orbbec::setIRAutoExposure(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irAutoExposure = value;
		return setBoolProperty(OB_PROP_IR_AUTO_EXPOSURE_BOOL, value);
	}

	bool _Orbbec::setIRExposure(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irExposure = value;
		return setIntProperty(OB_PROP_IR_EXPOSURE_INT, value);
	}

	bool _Orbbec::setIRGain(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irGain = value;
		return setIntProperty(OB_PROP_IR_GAIN_INT, value);
	}

	bool _Orbbec::setIRChannelDataSource(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_irChannelDataSource = value;
		return setIntProperty(OB_PROP_IR_CHANNEL_DATA_SOURCE_INT, value);
	}

	bool _Orbbec::setDepthRMFilter(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthRMFilter = value;
		return setBoolProperty(OB_PROP_DEPTH_RM_FILTER_BOOL, value);
	}

	bool _Orbbec::setColorAEMaxGain(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorAEMaxGain = value;
		return setIntProperty(OB_PROP_COLOR_AE_MAX_GAIN_INT, value);
	}

	bool _Orbbec::setDepthAutoExposurePriority(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthAutoExposurePriority = value;
		return setIntProperty(OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT, value);
	}

	bool _Orbbec::setSDKDisparityToDepth(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_sdkDisparityToDepth = value;
		return setBoolProperty(OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL, value);
	}

	bool _Orbbec::setSDKAccelFrameTransformed(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_sdkAccelFrameTransformed = value;
		return setBoolProperty(OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL, value);
	}

	bool _Orbbec::setSDKGyroFrameTransformed(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_sdkGyroFrameTransformed = value;
		return setBoolProperty(OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL, value);
	}

	bool _Orbbec::setDevicePerformanceMode(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_devicePerformanceMode = value;
		return setIntProperty(OB_PROP_DEVICE_PERFORMANCE_MODE_INT, value);
	}

	bool _Orbbec::setColorDenoisingLevel(int32_t value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorDenoisingLevel = value;
		return setIntProperty(OB_PROP_COLOR_DENOISING_LEVEL_INT, value);
	}

	bool _Orbbec::setMultiDeviceSyncConfig(const OBMultiDeviceSyncConfig &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_multiDeviceSyncConfig = value;
		NULL_F(m_spDev);
		try
		{
			IF_F(!(m_spDev->getSupportedMultiDeviceSyncModeBitmap() & value.syncMode));
			m_spDev->setMultiDeviceSyncConfig(value);
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec setMultiDeviceSyncConfig: ") + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::setDeviceIPAddrConfig(const OBNetIpConfig &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceIPAddrConfig = value;
		return setStructuredData(OB_STRUCT_DEVICE_IP_ADDR_CONFIG, &value, sizeof(value));
	}

	bool _Orbbec::setDepthHdrConfig(const OBHdrConfig &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthHdrConfig = value;
		return setStructuredData(OB_STRUCT_DEPTH_HDR_CONFIG, &value, sizeof(value));
	}

	bool _Orbbec::setColorAEROI(const OBRegionOfInterest &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorAEROI = value;
		return setStructuredData(OB_STRUCT_COLOR_AE_ROI, &value, sizeof(value));
	}

	bool _Orbbec::setDepthAEROI(const OBRegionOfInterest &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthAEROI = value;
		return setStructuredData(OB_STRUCT_DEPTH_AE_ROI, &value, sizeof(value));
	}

	bool _Orbbec::setDispOffsetConfig(const OBDispOffsetConfig &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_dispOffsetConfig = value;
		return setStructuredData(OB_STRUCT_DISP_OFFSET_CONFIG, &value, sizeof(value));
	}

	bool _Orbbec::setPresetResolutionConfig(const OBPresetResolutionConfig &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_presetResolutionConfig = value;
		return setStructuredData(OB_STRUCT_PRESET_RESOLUTION_CONFIG, &value, sizeof(value));
	}

	bool _Orbbec::setDeviceIPAddrConfigV2(const OBNetIpConfigV2 &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_deviceIPAddrConfigV2 = value;
		return setStructuredData(OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2, &value, sizeof(value));
	}

	bool _Orbbec::setTimestampResetConfig(const OBDeviceTimestampResetConfig &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_timestampResetConfig = value;
		NULL_F(m_spDev);
		try
		{
			m_spDev->setTimestampResetConfig(value);
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec setTimestampResetConfig: ") + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::switchDepthWorkMode(const string &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_depthWorkMode = value;
		NULL_F(m_spDev);
		IF_F(value.empty());
		try
		{
			return m_spDev->switchDepthWorkMode(value.c_str()) == OB_STATUS_OK;
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec switchDepthWorkMode: ") + e.what());
			return false;
		}
	}

	bool _Orbbec::switchColorPreset(const string &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_colorPreset = value;
		NULL_F(m_spDev);
		IF_F(value.empty());
		try
		{
			IF_F(!m_spDev->isColorPresetSupported());
			m_spDev->switchColorPreset(value.c_str());
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec switchColorPreset: ") + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::loadPreset(const string &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_preset = value;
		NULL_F(m_spDev);
		IF_F(value.empty());
		try
		{
			m_spDev->loadPreset(value.c_str());
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec loadPreset: ") + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::loadPresetFromJsonFile(const string &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_presetJsonFile = value;
		NULL_F(m_spDev);
		IF_F(value.empty());
		try
		{
			m_spDev->loadPresetFromJsonFile(value.c_str());
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec loadPresetFromJsonFile: ") + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::loadFrameInterleave(const string &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_frameInterleave = value;
		NULL_F(m_spDev);
		IF_F(value.empty());
		try
		{
			IF_F(!m_spDev->isFrameInterleaveSupported());
			m_spDev->loadFrameInterleave(value.c_str());
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec loadFrameInterleave: ") + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::enableGlobalTimestamp(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_globalTimestamp = value;
		NULL_F(m_spDev);
		try
		{
			IF_F(!m_spDev->isGlobalTimestampSupported());
			m_spDev->enableGlobalTimestamp(value);
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec enableGlobalTimestamp: ") + e.what());
			return false;
		}
		return true;
	}

	bool _Orbbec::enableFirmwareLog(bool value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxDevice);
		m_orbbecCtrl.m_firmwareLog = value;
		NULL_F(m_spDev);
		try
		{
			m_spDev->enableFirmwareLog(value);
		}
		catch (const ob::Error &e)
		{
			LOG_E(string("Orbbec enableFirmwareLog: ") + e.what());
			return false;
		}
		return true;
	}
}
