/*
 * _Orbbec.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_Orbbec.h"
#include <stdexcept>

// SDK C structures do not provide JSON conversions. Require complete objects to
// avoid writing zero-filled, partially configured exposure or network settings.
static void from_json(const json &j, OBMultiDeviceSyncConfig &value)
{
	value = {};
	if (!jKv(j, "syncMode", value.syncMode))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.syncMode");
	if (!jKv(j, "depthDelayUs", value.depthDelayUs))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.depthDelayUs");
	if (!jKv(j, "colorDelayUs", value.colorDelayUs))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.colorDelayUs");
	if (!jKv(j, "trigger2ImageDelayUs", value.trigger2ImageDelayUs))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.trigger2ImageDelayUs");
	if (!jKv(j, "triggerOutEnable", value.triggerOutEnable))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.triggerOutEnable");
	if (!jKv(j, "triggerOutDelayUs", value.triggerOutDelayUs))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.triggerOutDelayUs");
	if (!jKv(j, "framesPerTrigger", value.framesPerTrigger))
		throw std::invalid_argument("Missing or invalid OBMultiDeviceSyncConfig.framesPerTrigger");
}

static void from_json(const json &j, OBNetIpConfig &value)
{
	value = {};
	if (!jKv(j, "dhcp", value.dhcp))
		throw std::invalid_argument("Missing or invalid OBNetIpConfig.dhcp");
	{
		std::vector<int> octets;
		if (!jKv(j, "address", octets) || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfig.address requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (octets[i] < 0 || octets[i] > 255)
				throw std::invalid_argument("Invalid IPv4 octet in address");
			value.address[i] = static_cast<uint8_t>(octets[i]);
		}
	}
	{
		std::vector<int> octets;
		if (!jKv(j, "mask", octets) || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfig.mask requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (octets[i] < 0 || octets[i] > 255)
				throw std::invalid_argument("Invalid IPv4 octet in mask");
			value.mask[i] = static_cast<uint8_t>(octets[i]);
		}
	}
	{
		std::vector<int> octets;
		if (!jKv(j, "gateway", octets) || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfig.gateway requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (octets[i] < 0 || octets[i] > 255)
				throw std::invalid_argument("Invalid IPv4 octet in gateway");
			value.gateway[i] = static_cast<uint8_t>(octets[i]);
		}
	}
}

static void from_json(const json &j, OBHdrConfig &value)
{
	value = {};
	if (!jKv(j, "enable", value.enable))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.enable");
	if (!jKv(j, "sequence_name", value.sequence_name))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.sequence_name");
	if (!jKv(j, "exposure_1", value.exposure_1))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.exposure_1");
	if (!jKv(j, "gain_1", value.gain_1))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.gain_1");
	if (!jKv(j, "exposure_2", value.exposure_2))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.exposure_2");
	if (!jKv(j, "gain_2", value.gain_2))
		throw std::invalid_argument("Missing or invalid OBHdrConfig.gain_2");
}

static void from_json(const json &j, OBRegionOfInterest &value)
{
	value = {};
	if (!jKv(j, "x0_left", value.x0_left))
		throw std::invalid_argument("Missing or invalid OBRegionOfInterest.x0_left");
	if (!jKv(j, "y0_top", value.y0_top))
		throw std::invalid_argument("Missing or invalid OBRegionOfInterest.y0_top");
	if (!jKv(j, "x1_right", value.x1_right))
		throw std::invalid_argument("Missing or invalid OBRegionOfInterest.x1_right");
	if (!jKv(j, "y1_bottom", value.y1_bottom))
		throw std::invalid_argument("Missing or invalid OBRegionOfInterest.y1_bottom");
}

static void from_json(const json &j, OBDispOffsetConfig &value)
{
	value = {};
	if (!jKv(j, "enable", value.enable))
		throw std::invalid_argument("Missing or invalid OBDispOffsetConfig.enable");
	if (!jKv(j, "offset0", value.offset0))
		throw std::invalid_argument("Missing or invalid OBDispOffsetConfig.offset0");
	if (!jKv(j, "offset1", value.offset1))
		throw std::invalid_argument("Missing or invalid OBDispOffsetConfig.offset1");
}

static void from_json(const json &j, OBPresetResolutionConfig &value)
{
	value = {};
	if (!jKv(j, "width", value.width))
		throw std::invalid_argument("Missing or invalid OBPresetResolutionConfig.width");
	if (!jKv(j, "height", value.height))
		throw std::invalid_argument("Missing or invalid OBPresetResolutionConfig.height");
	if (!jKv(j, "irDecimationFactor", value.irDecimationFactor))
		throw std::invalid_argument("Missing or invalid OBPresetResolutionConfig.irDecimationFactor");
	if (!jKv(j, "depthDecimationFactor", value.depthDecimationFactor))
		throw std::invalid_argument("Missing or invalid OBPresetResolutionConfig.depthDecimationFactor");
}

static void from_json(const json &j, OBNetIpConfigV2 &value)
{
	value = {};
	if (!jKv(j, "flags", value.flags))
		throw std::invalid_argument("Missing or invalid OBNetIpConfigV2.flags");
	{
		std::vector<int> octets;
		if (!jKv(j, "address", octets) || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfigV2.address requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (octets[i] < 0 || octets[i] > 255)
				throw std::invalid_argument("Invalid IPv4 octet in address");
			value.address[i] = static_cast<uint8_t>(octets[i]);
		}
	}
	{
		std::vector<int> octets;
		if (!jKv(j, "mask", octets) || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfigV2.mask requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (octets[i] < 0 || octets[i] > 255)
				throw std::invalid_argument("Invalid IPv4 octet in mask");
			value.mask[i] = static_cast<uint8_t>(octets[i]);
		}
	}
	{
		std::vector<int> octets;
		if (!jKv(j, "gateway", octets) || octets.size() != 4)
			throw std::invalid_argument("OBNetIpConfigV2.gateway requires four IPv4 octets");
		for (size_t i = 0; i < octets.size(); ++i)
		{
			if (octets[i] < 0 || octets[i] > 255)
				throw std::invalid_argument("Invalid IPv4 octet in gateway");
			value.gateway[i] = static_cast<uint8_t>(octets[i]);
		}
	}
}

static void from_json(const json &j, OBDeviceTimestampResetConfig &value)
{
	value = {};
	if (!jKv(j, "enable", value.enable))
		throw std::invalid_argument("Missing or invalid OBDeviceTimestampResetConfig.enable");
	if (!jKv(j, "timestamp_reset_delay_us", value.timestamp_reset_delay_us))
		throw std::invalid_argument("Missing or invalid OBDeviceTimestampResetConfig.timestamp_reset_delay_us");
	if (!jKv(j, "timestamp_reset_signal_output_enable", value.timestamp_reset_signal_output_enable))
		throw std::invalid_argument("Missing or invalid OBDeviceTimestampResetConfig.timestamp_reset_signal_output_enable");
}

namespace
{
	using ::jKv;

	template <typename T>
	bool jKv(const json &j, const string &key, std::optional<T> &value)
	{
		if (!j.is_object() || !j.contains(key))
			return false;
		if (j.at(key).is_null())
		{
			value.reset();
			return true;
		}

		T configured{};
		if (!::jKv(j, key, configured))
			throw std::invalid_argument("Invalid Orbbec option: " + key);
		value = configured;
		return true;
	}
}

namespace kai
{

	_Orbbec::_Orbbec()
	{
	}

	_Orbbec::~_Orbbec()
	{
		close();
	}

	bool _Orbbec::init(const json &j)
	{
		IF_F(!_RGBDbase::init(j));

		jKv(j, "SN", m_SN);
		jKv(j, "tOutMs", m_tOutMs);

		try
		{
			jKv(j, "OB_PROP_LDP_BOOL", m_orbbecCtrl.m_ldp);
			jKv(j, "OB_PROP_LASER_BOOL", m_orbbecCtrl.m_laser);
			jKv(j, "OB_PROP_FLOOD_BOOL", m_orbbecCtrl.m_flood);
			jKv(j, "OB_PROP_TEMPERATURE_COMPENSATION_BOOL", m_orbbecCtrl.m_temperatureCompensation);
			jKv(j, "OB_PROP_DEPTH_MIRROR_BOOL", m_orbbecCtrl.m_depthMirror);
			jKv(j, "OB_PROP_DEPTH_FLIP_BOOL", m_orbbecCtrl.m_depthFlip);
			jKv(j, "OB_PROP_DEPTH_HOLEFILTER_BOOL", m_orbbecCtrl.m_depthHolefilter);
			jKv(j, "OB_PROP_IR_MIRROR_BOOL", m_orbbecCtrl.m_irMirror);
			jKv(j, "OB_PROP_IR_FLIP_BOOL", m_orbbecCtrl.m_irFlip);
			jKv(j, "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL", m_orbbecCtrl.m_depthNoiseRemovalFilter);
			jKv(j, "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT", m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff);
			jKv(j, "OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT", m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize);
			jKv(j, "OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL", m_orbbecCtrl.m_depthAlignHardware);
			jKv(j, "OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT", m_orbbecCtrl.m_depthAlignHardwareMode);
			jKv(j, "OB_PROP_DEPTH_PRECISION_LEVEL_INT", m_orbbecCtrl.m_depthPrecisionLevel);
			jKv(j, "OB_PROP_COLOR_MIRROR_BOOL", m_orbbecCtrl.m_colorMirror);
			jKv(j, "OB_PROP_COLOR_FLIP_BOOL", m_orbbecCtrl.m_colorFlip);
			jKv(j, "OB_PROP_DISPARITY_TO_DEPTH_BOOL", m_orbbecCtrl.m_disparityToDepth);
			jKv(j, "OB_PROP_WATCHDOG_BOOL", m_orbbecCtrl.m_watchdog);
			jKv(j, "OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL", m_orbbecCtrl.m_externalSignalReset);
			jKv(j, "OB_PROP_HEARTBEAT_BOOL", m_orbbecCtrl.m_heartbeat);
			jKv(j, "OB_PROP_LASER_POWER_LEVEL_CONTROL_INT", m_orbbecCtrl.m_laserPowerLevelControl);
			jKv(j, "OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL", m_orbbecCtrl.m_timerResetTriggerOutEnable);
			jKv(j, "OB_PROP_TIMER_RESET_DELAY_US_INT", m_orbbecCtrl.m_timerResetDelayUs);
			jKv(j, "OB_PROP_IR_RIGHT_MIRROR_BOOL", m_orbbecCtrl.m_irRightMirror);
			jKv(j, "OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT", m_orbbecCtrl.m_captureImageFrameNumber);
			jKv(j, "OB_PROP_IR_RIGHT_FLIP_BOOL", m_orbbecCtrl.m_irRightFlip);
			jKv(j, "OB_PROP_COLOR_ROTATE_INT", m_orbbecCtrl.m_colorRotate);
			jKv(j, "OB_PROP_IR_ROTATE_INT", m_orbbecCtrl.m_irRotate);
			jKv(j, "OB_PROP_IR_RIGHT_ROTATE_INT", m_orbbecCtrl.m_irRightRotate);
			jKv(j, "OB_PROP_DEPTH_ROTATE_INT", m_orbbecCtrl.m_depthRotate);
			jKv(j, "OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL", m_orbbecCtrl.m_syncSignalTriggerOut);
			jKv(j, "OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL", m_orbbecCtrl.m_deviceUSB2RepeatIdentify);
			jKv(j, "OB_PROP_LASER_ALWAYS_ON_BOOL", m_orbbecCtrl.m_laserAlwaysOn);
			jKv(j, "OB_PROP_LASER_ON_OFF_PATTERN_INT", m_orbbecCtrl.m_laserOnOffPattern);
			jKv(j, "OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT", m_orbbecCtrl.m_depthUnitFlexibleAdjustment);
			jKv(j, "OB_PROP_LASER_CONTROL_INT", m_orbbecCtrl.m_laserControl);
			jKv(j, "OB_PROP_IR_BRIGHTNESS_INT", m_orbbecCtrl.m_irBrightness);
			jKv(j, "OB_PROP_COLOR_AE_MAX_EXPOSURE_INT", m_orbbecCtrl.m_colorAEMaxExposure);
			jKv(j, "OB_PROP_IR_AE_MAX_EXPOSURE_INT", m_orbbecCtrl.m_irAEMaxExposure);
			jKv(j, "OB_PROP_DISP_SEARCH_RANGE_MODE_INT", m_orbbecCtrl.m_dispSearchRangeMode);
			jKv(j, "OB_PROP_DISP_SEARCH_OFFSET_INT", m_orbbecCtrl.m_dispSearchOffset);
			jKv(j, "OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL", m_orbbecCtrl.m_cpuTemperatureCalibration);
			jKv(j, "OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT", m_orbbecCtrl.m_frameInterleaveConfigIndex);
			jKv(j, "OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL", m_orbbecCtrl.m_frameInterleaveEnable);
			jKv(j, "OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT", m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay);
			jKv(j, "OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL", m_orbbecCtrl.m_onChipCalibrationEnable);
			jKv(j, "OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL", m_orbbecCtrl.m_hwNoiseRemoveFilterEnable);
			jKv(j, "OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT", m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold);
			jKv(j, "OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL", m_orbbecCtrl.m_deviceAutoCaptureEnable);
			jKv(j, "OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT", m_orbbecCtrl.m_deviceAutoCaptureIntervalTime);
			jKv(j, "OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL", m_orbbecCtrl.m_devicePTPClockSyncEnable);
			jKv(j, "OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL", m_orbbecCtrl.m_confidenceStreamFilter);
			jKv(j, "OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT", m_orbbecCtrl.m_confidenceStreamFilterThreshold);
			jKv(j, "OB_PROP_CONFIDENCE_MIRROR_BOOL", m_orbbecCtrl.m_confidenceMirror);
			jKv(j, "OB_PROP_CONFIDENCE_FLIP_BOOL", m_orbbecCtrl.m_confidenceFlip);
			jKv(j, "OB_PROP_CONFIDENCE_ROTATE_INT", m_orbbecCtrl.m_confidenceRotate);
			jKv(j, "OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT", m_orbbecCtrl.m_intraCameraSyncReference);
			jKv(j, "OB_PROP_COLOR_RIGHT_ROTATE_INT", m_orbbecCtrl.m_colorRightRotate);
			jKv(j, "OB_PROP_COLOR_RIGHT_MIRROR_BOOL", m_orbbecCtrl.m_colorRightMirror);
			jKv(j, "OB_PROP_COLOR_RIGHT_FLIP_BOOL", m_orbbecCtrl.m_colorRightFlip);
			jKv(j, "OB_PROP_DEVICE_AE_REFERENCE_INT", m_orbbecCtrl.m_deviceAEReference);
			jKv(j, "OB_PROP_DEVICE_AE_STRATEGY_INT", m_orbbecCtrl.m_deviceAEStrategy);
			jKv(j, "OB_PROP_COLOR_ROI_BRIGHTNESS_INT", m_orbbecCtrl.m_colorROIBrightness);
			jKv(j, "OB_PROP_COLOR_LEFT_ROTATE_INT", m_orbbecCtrl.m_colorLeftRotate);
			jKv(j, "OB_PROP_COLOR_LEFT_MIRROR_BOOL", m_orbbecCtrl.m_colorLeftMirror);
			jKv(j, "OB_PROP_COLOR_LEFT_FLIP_BOOL", m_orbbecCtrl.m_colorLeftFlip);
			jKv(j, "OB_PROP_COLOR_PRESET_PRIORITY_INT", m_orbbecCtrl.m_colorPresetPriority);
			jKv(j, "OB_PROP_DEVICE_NETWORK_LLA_BOOL", m_orbbecCtrl.m_deviceNetworkLLA);
			jKv(j, "OB_PROP_COLOR_ANTI_FLICKER_BOOL", m_orbbecCtrl.m_colorAntiFlicker);
			jKv(j, "OB_PROP_DEVICE_IP_MODE_INT", m_orbbecCtrl.m_deviceIPMode);
			jKv(j, "OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT", m_orbbecCtrl.m_dhcpAssignIPTimeout);
			jKv(j, "OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT", m_orbbecCtrl.m_usbSyncVoltageLevel);
			jKv(j, "OB_PROP_FPS_BOOST_BOOL", m_orbbecCtrl.m_fpsBoost);
			jKv(j, "OB_PROP_MJPEG_QUALITY_INT", m_orbbecCtrl.m_mjpegQuality);
			jKv(j, "OB_PROP_COLOR_AUTO_EXPOSURE_BOOL", m_orbbecCtrl.m_colorAutoExposure);
			jKv(j, "OB_PROP_COLOR_EXPOSURE_INT", m_orbbecCtrl.m_colorExposure);
			jKv(j, "OB_PROP_COLOR_GAIN_INT", m_orbbecCtrl.m_colorGain);
			jKv(j, "OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL", m_orbbecCtrl.m_colorAutoWhiteBalance);
			jKv(j, "OB_PROP_COLOR_WHITE_BALANCE_INT", m_orbbecCtrl.m_colorWhiteBalance);
			jKv(j, "OB_PROP_COLOR_BRIGHTNESS_INT", m_orbbecCtrl.m_colorBrightness);
			jKv(j, "OB_PROP_COLOR_SHARPNESS_INT", m_orbbecCtrl.m_colorSharpness);
			jKv(j, "OB_PROP_COLOR_SATURATION_INT", m_orbbecCtrl.m_colorSaturation);
			jKv(j, "OB_PROP_COLOR_CONTRAST_INT", m_orbbecCtrl.m_colorContrast);
			jKv(j, "OB_PROP_COLOR_GAMMA_INT", m_orbbecCtrl.m_colorGamma);
			jKv(j, "OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT", m_orbbecCtrl.m_colorAutoExposurePriority);
			jKv(j, "OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT", m_orbbecCtrl.m_colorBacklightCompensation);
			jKv(j, "OB_PROP_COLOR_HUE_INT", m_orbbecCtrl.m_colorHue);
			jKv(j, "OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT", m_orbbecCtrl.m_colorPowerLineFrequency);
			jKv(j, "OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL", m_orbbecCtrl.m_depthAutoExposure);
			jKv(j, "OB_PROP_DEPTH_EXPOSURE_INT", m_orbbecCtrl.m_depthExposure);
			jKv(j, "OB_PROP_DEPTH_GAIN_INT", m_orbbecCtrl.m_depthGain);
			jKv(j, "OB_PROP_IR_AUTO_EXPOSURE_BOOL", m_orbbecCtrl.m_irAutoExposure);
			jKv(j, "OB_PROP_IR_EXPOSURE_INT", m_orbbecCtrl.m_irExposure);
			jKv(j, "OB_PROP_IR_GAIN_INT", m_orbbecCtrl.m_irGain);
			jKv(j, "OB_PROP_IR_CHANNEL_DATA_SOURCE_INT", m_orbbecCtrl.m_irChannelDataSource);
			jKv(j, "OB_PROP_DEPTH_RM_FILTER_BOOL", m_orbbecCtrl.m_depthRMFilter);
			jKv(j, "OB_PROP_COLOR_AE_MAX_GAIN_INT", m_orbbecCtrl.m_colorAEMaxGain);
			jKv(j, "OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT", m_orbbecCtrl.m_depthAutoExposurePriority);
			jKv(j, "OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL", m_orbbecCtrl.m_sdkDisparityToDepth);
			jKv(j, "OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL", m_orbbecCtrl.m_sdkAccelFrameTransformed);
			jKv(j, "OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL", m_orbbecCtrl.m_sdkGyroFrameTransformed);
			jKv(j, "OB_PROP_DEVICE_PERFORMANCE_MODE_INT", m_orbbecCtrl.m_devicePerformanceMode);
			jKv(j, "OB_PROP_COLOR_DENOISING_LEVEL_INT", m_orbbecCtrl.m_colorDenoisingLevel);
			jKv(j, "OB_STRUCT_MULTI_DEVICE_SYNC_CONFIG", m_orbbecCtrl.m_multiDeviceSyncConfig);
			jKv(j, "OB_STRUCT_DEVICE_IP_ADDR_CONFIG", m_orbbecCtrl.m_deviceIPAddrConfig);
			jKv(j, "OB_STRUCT_DEPTH_HDR_CONFIG", m_orbbecCtrl.m_depthHdrConfig);
			jKv(j, "OB_STRUCT_COLOR_AE_ROI", m_orbbecCtrl.m_colorAEROI);
			jKv(j, "OB_STRUCT_DEPTH_AE_ROI", m_orbbecCtrl.m_depthAEROI);
			jKv(j, "OB_STRUCT_DISP_OFFSET_CONFIG", m_orbbecCtrl.m_dispOffsetConfig);
			jKv(j, "OB_STRUCT_PRESET_RESOLUTION_CONFIG", m_orbbecCtrl.m_presetResolutionConfig);
			jKv(j, "OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2", m_orbbecCtrl.m_deviceIPAddrConfigV2);
			jKv(j, "obTimestampResetConfig", m_orbbecCtrl.m_timestampResetConfig);
			jKv(j, "obDepthWorkMode", m_orbbecCtrl.m_depthWorkMode);
			jKv(j, "obColorPreset", m_orbbecCtrl.m_colorPreset);
			jKv(j, "obPreset", m_orbbecCtrl.m_preset);
			jKv(j, "obPresetJsonFile", m_orbbecCtrl.m_presetJsonFile);
			jKv(j, "obFrameInterleave", m_orbbecCtrl.m_frameInterleave);
			jKv(j, "obGlobalTimestamp", m_orbbecCtrl.m_globalTimestamp);
			jKv(j, "obFirmwareLog", m_orbbecCtrl.m_firmwareLog);
		}
		catch (const std::exception &e)
		{
			LOG_E(string("Invalid Orbbec controls: ") + e.what());
			return false;
		}
		DEL(m_pTpp);
		m_pTpp = createThread(jK(j, "threadPP"), "threadPP");
		NULL_F(m_pTpp);

		return true;
	}

	bool _Orbbec::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_RGBDbase::link(j, pM));

		return true;
	}

	bool _Orbbec::open(void)
	{
		IF__(m_bOpened, true);

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

		// IMU streams
		if (m_bIMU)
		{
			m_spConfig->enableAccelStream(OB_ACCEL_FS_4g, OB_SAMPLE_RATE_200_HZ);
			m_spConfig->enableGyroStream(OB_GYRO_FS_1000dps, OB_SAMPLE_RATE_200_HZ);
		}

		// For point cloud generation
		if (m_bPCLrgb)
		{
			m_spConfig->setFrameAggregateOutputMode(OB_FRAME_AGGREGATE_OUTPUT_ALL_TYPE_FRAME_REQUIRE);
			m_spConfig->setAlignMode(ALIGN_D2C_HW_MODE);
			m_spPipe->enableFrameSync();
		}

		m_spPipe->start(m_spConfig);

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

		m_tDus = 0;
		m_dtDus = 0;
		m_tRGBus = 0;
		m_dtRGBus = 0;

		m_bOpened = true;
		return true;
	}

	void _Orbbec::close(void)
	{
		IF_(!m_bOpened);

		m_spPipe->stop();
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

	bool _Orbbec::check(void)
	{
		return this->_RGBDbase::check();
	}

	void _Orbbec::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpened)
			{
				if (!open())
				{
					LOG_E("Cannot open Orbbec");
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			if (updateOrbbec())
			{
				m_pTpp->run();
			}
			else
			{
				close();
			}
		}
	}

	bool _Orbbec::updateOrbbec(void)
	{
		IF_F(!check());

		shared_ptr<ob::FrameSet> spFS = m_spPipe->waitForFrameset(m_tOutMs);
		NULL_F(spFS);

		// IMU, fast stream
		if (auto g = spFS->getFrame(OB_FRAME_GYRO))
		{
			auto gf = g->as<ob::GyroFrame>();
			if (gf)
			{
				auto v = gf->value();
				if (m_pIMU)
					m_pIMU->addGyro(frameTsUs_(g), {v.x, v.y, v.z});
			}
		}

		if (auto a = spFS->getFrame(OB_FRAME_ACCEL))
		{
			auto af = a->as<ob::AccelFrame>();
			if (af)
			{
				auto v = af->value();
				if (m_pIMU)
					m_pIMU->addAcc(frameTsUs_(a), {v.x, v.y, v.z});
			}
		}

		// Images, slow stream
		shared_ptr<ob::Frame> spFrameRGB = nullptr;
		shared_ptr<ob::Frame> spFrameD = nullptr;

		if (m_bRGB)
		{
			spFrameRGB = spFS->getFrame(OB_FRAME_COLOR);
			if (spFrameRGB)
			{
				m_mRGB = Mat(m_vSizeRGB.y(), m_vSizeRGB.x(), CV_8UC3, spFrameRGB->getData());
				uint64_t tRGBus = frameTsUs_(spFrameRGB);
				m_dtRGBus = tRGBus - m_tRGBus;
				m_tRGBus = tRGBus;
			}
		}

		if (m_bDepth)
		{
			spFrameD = spFS->getFrame(OB_FRAME_DEPTH);
			if (spFrameD)
			{
				m_mDepth = Mat(m_vSizeD.y(), m_vSizeD.x(), CV_16UC1, spFrameD->getData());
				uint64_t tDus = frameTsUs_(spFrameD);
				m_dtDus = tDus - m_tDus;
				m_tDus = tDus;
			}
		}

		// Point cloud
		if (m_bPCLrgb && spFrameRGB && spFrameD)
		{
			m_spFrame = m_spPCF->process(spFS);
		}
		else if (m_bPCL && spFrameD)
		{
			m_spFrame = m_spPCF->process(spFS);
		}

		return true;
	}

	void _Orbbec::updateTPP(void)
	{
		while (m_pTpp->bRun())
		{
			m_pTpp->sleepT(0);

			updatePCL();
		}
	}

	void _Orbbec::updatePCL(void)
	{
#ifdef WITH_UNIVERSE
		const auto spFrame = m_spFrame;
		NULL_(spFrame);
		NULL_(m_pPCL);

		const auto format = spFrame->getFormat();
		IF_(format != OB_FORMAT_POINT && format != OB_FORMAT_RGB_POINT);

		// The SDK scale converts point coordinates to millimeters.
		const float s_b = spFrame->as<ob::PointsFrame>()->getCoordinateValueScale() * 0.001f;
		const uint64_t tDus = frameTsUs_(spFrame);

		m_pPCL->frameStart();

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
				m_pPCL->add(vP, vC, tDus);
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
				m_pPCL->add(vP, vC, tDus);
			}
		}

		m_pPCL->frameStop();
#endif
	}

	void _Orbbec::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_RGBDbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("tDus = " + li2str(m_tDus) + ", dtDus = " + li2str(m_dtDus));
		pC->addMsg("tRGBus = " + li2str(m_tRGBus) + ", dtRGBus = " + li2str(m_dtRGBus));
	}

	OrbbecCtrl _Orbbec::getCamCtrl(void) const
	{
		return m_orbbecCtrl;
	}

	bool _Orbbec::setCamCtrl(const OrbbecCtrl &camCtrl)
	{
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
		m_orbbecCtrl.m_ldp = value;
		return setBoolProperty(OB_PROP_LDP_BOOL, value);
	}

	bool _Orbbec::setLaser(bool value)
	{
		m_orbbecCtrl.m_laser = value;
		return setBoolProperty(OB_PROP_LASER_BOOL, value);
	}

	bool _Orbbec::setFlood(bool value)
	{
		m_orbbecCtrl.m_flood = value;
		return setBoolProperty(OB_PROP_FLOOD_BOOL, value);
	}

	bool _Orbbec::setTemperatureCompensation(bool value)
	{
		m_orbbecCtrl.m_temperatureCompensation = value;
		return setBoolProperty(OB_PROP_TEMPERATURE_COMPENSATION_BOOL, value);
	}

	bool _Orbbec::setDepthMirror(bool value)
	{
		m_orbbecCtrl.m_depthMirror = value;
		return setBoolProperty(OB_PROP_DEPTH_MIRROR_BOOL, value);
	}

	bool _Orbbec::setDepthFlip(bool value)
	{
		m_orbbecCtrl.m_depthFlip = value;
		return setBoolProperty(OB_PROP_DEPTH_FLIP_BOOL, value);
	}

	bool _Orbbec::setDepthHolefilter(bool value)
	{
		m_orbbecCtrl.m_depthHolefilter = value;
		return setBoolProperty(OB_PROP_DEPTH_HOLEFILTER_BOOL, value);
	}

	bool _Orbbec::setIRMirror(bool value)
	{
		m_orbbecCtrl.m_irMirror = value;
		return setBoolProperty(OB_PROP_IR_MIRROR_BOOL, value);
	}

	bool _Orbbec::setIRFlip(bool value)
	{
		m_orbbecCtrl.m_irFlip = value;
		return setBoolProperty(OB_PROP_IR_FLIP_BOOL, value);
	}

	bool _Orbbec::setDepthNoiseRemovalFilter(bool value)
	{
		m_orbbecCtrl.m_depthNoiseRemovalFilter = value;
		return setBoolProperty(OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL, value);
	}

	bool _Orbbec::setDepthNoiseRemovalFilterMaxDiff(int32_t value)
	{
		m_orbbecCtrl.m_depthNoiseRemovalFilterMaxDiff = value;
		return setIntProperty(OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT, value);
	}

	bool _Orbbec::setDepthNoiseRemovalFilterMaxSpeckleSize(int32_t value)
	{
		m_orbbecCtrl.m_depthNoiseRemovalFilterMaxSpeckleSize = value;
		return setIntProperty(OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT, value);
	}

	bool _Orbbec::setDepthAlignHardware(bool value)
	{
		m_orbbecCtrl.m_depthAlignHardware = value;
		return setBoolProperty(OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL, value);
	}

	bool _Orbbec::setDepthAlignHardwareMode(int32_t value)
	{
		m_orbbecCtrl.m_depthAlignHardwareMode = value;
		return setIntProperty(OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT, value);
	}

	bool _Orbbec::setDepthPrecisionLevel(int32_t value)
	{
		m_orbbecCtrl.m_depthPrecisionLevel = value;
		return setIntProperty(OB_PROP_DEPTH_PRECISION_LEVEL_INT, value);
	}

	bool _Orbbec::setColorMirror(bool value)
	{
		m_orbbecCtrl.m_colorMirror = value;
		return setBoolProperty(OB_PROP_COLOR_MIRROR_BOOL, value);
	}

	bool _Orbbec::setColorFlip(bool value)
	{
		m_orbbecCtrl.m_colorFlip = value;
		return setBoolProperty(OB_PROP_COLOR_FLIP_BOOL, value);
	}

	bool _Orbbec::setDisparityToDepth(bool value)
	{
		m_orbbecCtrl.m_disparityToDepth = value;
		return setBoolProperty(OB_PROP_DISPARITY_TO_DEPTH_BOOL, value);
	}

	bool _Orbbec::setWatchdog(bool value)
	{
		m_orbbecCtrl.m_watchdog = value;
		return setBoolProperty(OB_PROP_WATCHDOG_BOOL, value);
	}

	bool _Orbbec::setExternalSignalReset(bool value)
	{
		m_orbbecCtrl.m_externalSignalReset = value;
		return setBoolProperty(OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL, value);
	}

	bool _Orbbec::setHeartbeat(bool value)
	{
		m_orbbecCtrl.m_heartbeat = value;
		return setBoolProperty(OB_PROP_HEARTBEAT_BOOL, value);
	}

	bool _Orbbec::setLaserPowerLevelControl(int32_t value)
	{
		m_orbbecCtrl.m_laserPowerLevelControl = value;
		return setIntProperty(OB_PROP_LASER_POWER_LEVEL_CONTROL_INT, value);
	}

	bool _Orbbec::setTimerResetTriggerOutEnable(bool value)
	{
		m_orbbecCtrl.m_timerResetTriggerOutEnable = value;
		return setBoolProperty(OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL, value);
	}

	bool _Orbbec::setTimerResetDelayUs(int32_t value)
	{
		m_orbbecCtrl.m_timerResetDelayUs = value;
		return setIntProperty(OB_PROP_TIMER_RESET_DELAY_US_INT, value);
	}

	bool _Orbbec::setIRRightMirror(bool value)
	{
		m_orbbecCtrl.m_irRightMirror = value;
		return setBoolProperty(OB_PROP_IR_RIGHT_MIRROR_BOOL, value);
	}

	bool _Orbbec::setCaptureImageFrameNumber(int32_t value)
	{
		m_orbbecCtrl.m_captureImageFrameNumber = value;
		return setIntProperty(OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT, value);
	}

	bool _Orbbec::setIRRightFlip(bool value)
	{
		m_orbbecCtrl.m_irRightFlip = value;
		return setBoolProperty(OB_PROP_IR_RIGHT_FLIP_BOOL, value);
	}

	bool _Orbbec::setColorRotate(int32_t value)
	{
		m_orbbecCtrl.m_colorRotate = value;
		return setIntProperty(OB_PROP_COLOR_ROTATE_INT, value);
	}

	bool _Orbbec::setIRRotate(int32_t value)
	{
		m_orbbecCtrl.m_irRotate = value;
		return setIntProperty(OB_PROP_IR_ROTATE_INT, value);
	}

	bool _Orbbec::setIRRightRotate(int32_t value)
	{
		m_orbbecCtrl.m_irRightRotate = value;
		return setIntProperty(OB_PROP_IR_RIGHT_ROTATE_INT, value);
	}

	bool _Orbbec::setDepthRotate(int32_t value)
	{
		m_orbbecCtrl.m_depthRotate = value;
		return setIntProperty(OB_PROP_DEPTH_ROTATE_INT, value);
	}

	bool _Orbbec::setSyncSignalTriggerOut(bool value)
	{
		m_orbbecCtrl.m_syncSignalTriggerOut = value;
		return setBoolProperty(OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL, value);
	}

	bool _Orbbec::setDeviceUSB2RepeatIdentify(bool value)
	{
		m_orbbecCtrl.m_deviceUSB2RepeatIdentify = value;
		return setBoolProperty(OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL, value);
	}

	bool _Orbbec::setLaserAlwaysOn(bool value)
	{
		m_orbbecCtrl.m_laserAlwaysOn = value;
		return setBoolProperty(OB_PROP_LASER_ALWAYS_ON_BOOL, value);
	}

	bool _Orbbec::setLaserOnOffPattern(int32_t value)
	{
		m_orbbecCtrl.m_laserOnOffPattern = value;
		return setIntProperty(OB_PROP_LASER_ON_OFF_PATTERN_INT, value);
	}

	bool _Orbbec::setDepthUnitFlexibleAdjustment(float value)
	{
		m_orbbecCtrl.m_depthUnitFlexibleAdjustment = value;
		return setFloatProperty(OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT, value);
	}

	bool _Orbbec::setLaserControl(int32_t value)
	{
		m_orbbecCtrl.m_laserControl = value;
		return setIntProperty(OB_PROP_LASER_CONTROL_INT, value);
	}

	bool _Orbbec::setIRBrightness(int32_t value)
	{
		m_orbbecCtrl.m_irBrightness = value;
		return setIntProperty(OB_PROP_IR_BRIGHTNESS_INT, value);
	}

	bool _Orbbec::setColorAEMaxExposure(int32_t value)
	{
		m_orbbecCtrl.m_colorAEMaxExposure = value;
		return setIntProperty(OB_PROP_COLOR_AE_MAX_EXPOSURE_INT, value);
	}

	bool _Orbbec::setIRAEMaxExposure(int32_t value)
	{
		m_orbbecCtrl.m_irAEMaxExposure = value;
		return setIntProperty(OB_PROP_IR_AE_MAX_EXPOSURE_INT, value);
	}

	bool _Orbbec::setDispSearchRangeMode(int32_t value)
	{
		m_orbbecCtrl.m_dispSearchRangeMode = value;
		return setIntProperty(OB_PROP_DISP_SEARCH_RANGE_MODE_INT, value);
	}

	bool _Orbbec::setDispSearchOffset(int32_t value)
	{
		m_orbbecCtrl.m_dispSearchOffset = value;
		return setIntProperty(OB_PROP_DISP_SEARCH_OFFSET_INT, value);
	}

	bool _Orbbec::setCPUTemperatureCalibration(bool value)
	{
		m_orbbecCtrl.m_cpuTemperatureCalibration = value;
		return setBoolProperty(OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL, value);
	}

	bool _Orbbec::setFrameInterleaveConfigIndex(int32_t value)
	{
		m_orbbecCtrl.m_frameInterleaveConfigIndex = value;
		return setIntProperty(OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT, value);
	}

	bool _Orbbec::setFrameInterleaveEnable(bool value)
	{
		m_orbbecCtrl.m_frameInterleaveEnable = value;
		return setBoolProperty(OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL, value);
	}

	bool _Orbbec::setFrameInterleaveLaserPatternSyncDelay(int32_t value)
	{
		m_orbbecCtrl.m_frameInterleaveLaserPatternSyncDelay = value;
		return setIntProperty(OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT, value);
	}

	bool _Orbbec::setOnChipCalibrationEnable(bool value)
	{
		m_orbbecCtrl.m_onChipCalibrationEnable = value;
		return setBoolProperty(OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL, value);
	}

	bool _Orbbec::setHWNoiseRemoveFilterEnable(bool value)
	{
		m_orbbecCtrl.m_hwNoiseRemoveFilterEnable = value;
		return setBoolProperty(OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL, value);
	}

	bool _Orbbec::setHWNoiseRemoveFilterThreshold(float value)
	{
		m_orbbecCtrl.m_hwNoiseRemoveFilterThreshold = value;
		return setFloatProperty(OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT, value);
	}

	bool _Orbbec::setDeviceAutoCaptureEnable(bool value)
	{
		m_orbbecCtrl.m_deviceAutoCaptureEnable = value;
		return setBoolProperty(OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL, value);
	}

	bool _Orbbec::setDeviceAutoCaptureIntervalTime(int32_t value)
	{
		m_orbbecCtrl.m_deviceAutoCaptureIntervalTime = value;
		return setIntProperty(OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT, value);
	}

	bool _Orbbec::setDevicePTPClockSyncEnable(bool value)
	{
		m_orbbecCtrl.m_devicePTPClockSyncEnable = value;
		return setBoolProperty(OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL, value);
	}

	bool _Orbbec::setConfidenceStreamFilter(bool value)
	{
		m_orbbecCtrl.m_confidenceStreamFilter = value;
		return setBoolProperty(OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL, value);
	}

	bool _Orbbec::setConfidenceStreamFilterThreshold(int32_t value)
	{
		m_orbbecCtrl.m_confidenceStreamFilterThreshold = value;
		return setIntProperty(OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT, value);
	}

	bool _Orbbec::setConfidenceMirror(bool value)
	{
		m_orbbecCtrl.m_confidenceMirror = value;
		return setBoolProperty(OB_PROP_CONFIDENCE_MIRROR_BOOL, value);
	}

	bool _Orbbec::setConfidenceFlip(bool value)
	{
		m_orbbecCtrl.m_confidenceFlip = value;
		return setBoolProperty(OB_PROP_CONFIDENCE_FLIP_BOOL, value);
	}

	bool _Orbbec::setConfidenceRotate(int32_t value)
	{
		m_orbbecCtrl.m_confidenceRotate = value;
		return setIntProperty(OB_PROP_CONFIDENCE_ROTATE_INT, value);
	}

	bool _Orbbec::setIntraCameraSyncReference(int32_t value)
	{
		m_orbbecCtrl.m_intraCameraSyncReference = value;
		return setIntProperty(OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT, value);
	}

	bool _Orbbec::setColorRightRotate(int32_t value)
	{
		m_orbbecCtrl.m_colorRightRotate = value;
		return setIntProperty(OB_PROP_COLOR_RIGHT_ROTATE_INT, value);
	}

	bool _Orbbec::setColorRightMirror(bool value)
	{
		m_orbbecCtrl.m_colorRightMirror = value;
		return setBoolProperty(OB_PROP_COLOR_RIGHT_MIRROR_BOOL, value);
	}

	bool _Orbbec::setColorRightFlip(bool value)
	{
		m_orbbecCtrl.m_colorRightFlip = value;
		return setBoolProperty(OB_PROP_COLOR_RIGHT_FLIP_BOOL, value);
	}

	bool _Orbbec::setDeviceAEReference(int32_t value)
	{
		m_orbbecCtrl.m_deviceAEReference = value;
		return setIntProperty(OB_PROP_DEVICE_AE_REFERENCE_INT, value);
	}

	bool _Orbbec::setDeviceAEStrategy(int32_t value)
	{
		m_orbbecCtrl.m_deviceAEStrategy = value;
		return setIntProperty(OB_PROP_DEVICE_AE_STRATEGY_INT, value);
	}

	bool _Orbbec::setColorROIBrightness(int32_t value)
	{
		m_orbbecCtrl.m_colorROIBrightness = value;
		return setIntProperty(OB_PROP_COLOR_ROI_BRIGHTNESS_INT, value);
	}

	bool _Orbbec::setColorLeftRotate(int32_t value)
	{
		m_orbbecCtrl.m_colorLeftRotate = value;
		return setIntProperty(OB_PROP_COLOR_LEFT_ROTATE_INT, value);
	}

	bool _Orbbec::setColorLeftMirror(bool value)
	{
		m_orbbecCtrl.m_colorLeftMirror = value;
		return setBoolProperty(OB_PROP_COLOR_LEFT_MIRROR_BOOL, value);
	}

	bool _Orbbec::setColorLeftFlip(bool value)
	{
		m_orbbecCtrl.m_colorLeftFlip = value;
		return setBoolProperty(OB_PROP_COLOR_LEFT_FLIP_BOOL, value);
	}

	bool _Orbbec::setColorPresetPriority(int32_t value)
	{
		m_orbbecCtrl.m_colorPresetPriority = value;
		return setIntProperty(OB_PROP_COLOR_PRESET_PRIORITY_INT, value);
	}

	bool _Orbbec::setDeviceNetworkLLA(bool value)
	{
		m_orbbecCtrl.m_deviceNetworkLLA = value;
		return setBoolProperty(OB_PROP_DEVICE_NETWORK_LLA_BOOL, value);
	}

	bool _Orbbec::setColorAntiFlicker(bool value)
	{
		m_orbbecCtrl.m_colorAntiFlicker = value;
		return setBoolProperty(OB_PROP_COLOR_ANTI_FLICKER_BOOL, value);
	}

	bool _Orbbec::setDeviceIPMode(int32_t value)
	{
		m_orbbecCtrl.m_deviceIPMode = value;
		return setIntProperty(OB_PROP_DEVICE_IP_MODE_INT, value);
	}

	bool _Orbbec::setDHCPAssignIPTimeout(int32_t value)
	{
		m_orbbecCtrl.m_dhcpAssignIPTimeout = value;
		return setIntProperty(OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT, value);
	}

	bool _Orbbec::setUSBSyncVoltageLevel(int32_t value)
	{
		m_orbbecCtrl.m_usbSyncVoltageLevel = value;
		return setIntProperty(OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT, value);
	}

	bool _Orbbec::setFPSBoost(bool value)
	{
		m_orbbecCtrl.m_fpsBoost = value;
		return setBoolProperty(OB_PROP_FPS_BOOST_BOOL, value);
	}

	bool _Orbbec::setMJPEGQuality(int32_t value)
	{
		m_orbbecCtrl.m_mjpegQuality = value;
		return setIntProperty(OB_PROP_MJPEG_QUALITY_INT, value);
	}

	bool _Orbbec::setColorAutoExposure(bool value)
	{
		m_orbbecCtrl.m_colorAutoExposure = value;
		return setBoolProperty(OB_PROP_COLOR_AUTO_EXPOSURE_BOOL, value);
	}

	bool _Orbbec::setColorExposure(int32_t value)
	{
		m_orbbecCtrl.m_colorExposure = value;
		return setIntProperty(OB_PROP_COLOR_EXPOSURE_INT, value);
	}

	bool _Orbbec::setColorGain(int32_t value)
	{
		m_orbbecCtrl.m_colorGain = value;
		return setIntProperty(OB_PROP_COLOR_GAIN_INT, value);
	}

	bool _Orbbec::setColorAutoWhiteBalance(bool value)
	{
		m_orbbecCtrl.m_colorAutoWhiteBalance = value;
		return setBoolProperty(OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL, value);
	}

	bool _Orbbec::setColorWhiteBalance(int32_t value)
	{
		m_orbbecCtrl.m_colorWhiteBalance = value;
		return setIntProperty(OB_PROP_COLOR_WHITE_BALANCE_INT, value);
	}

	bool _Orbbec::setColorBrightness(int32_t value)
	{
		m_orbbecCtrl.m_colorBrightness = value;
		return setIntProperty(OB_PROP_COLOR_BRIGHTNESS_INT, value);
	}

	bool _Orbbec::setColorSharpness(int32_t value)
	{
		m_orbbecCtrl.m_colorSharpness = value;
		return setIntProperty(OB_PROP_COLOR_SHARPNESS_INT, value);
	}

	bool _Orbbec::setColorSaturation(int32_t value)
	{
		m_orbbecCtrl.m_colorSaturation = value;
		return setIntProperty(OB_PROP_COLOR_SATURATION_INT, value);
	}

	bool _Orbbec::setColorContrast(int32_t value)
	{
		m_orbbecCtrl.m_colorContrast = value;
		return setIntProperty(OB_PROP_COLOR_CONTRAST_INT, value);
	}

	bool _Orbbec::setColorGamma(int32_t value)
	{
		m_orbbecCtrl.m_colorGamma = value;
		return setIntProperty(OB_PROP_COLOR_GAMMA_INT, value);
	}

	bool _Orbbec::setColorAutoExposurePriority(int32_t value)
	{
		m_orbbecCtrl.m_colorAutoExposurePriority = value;
		return setIntProperty(OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT, value);
	}

	bool _Orbbec::setColorBacklightCompensation(int32_t value)
	{
		m_orbbecCtrl.m_colorBacklightCompensation = value;
		return setIntProperty(OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT, value);
	}

	bool _Orbbec::setColorHue(int32_t value)
	{
		m_orbbecCtrl.m_colorHue = value;
		return setIntProperty(OB_PROP_COLOR_HUE_INT, value);
	}

	bool _Orbbec::setColorPowerLineFrequency(int32_t value)
	{
		m_orbbecCtrl.m_colorPowerLineFrequency = value;
		return setIntProperty(OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT, value);
	}

	bool _Orbbec::setDepthAutoExposure(bool value)
	{
		m_orbbecCtrl.m_depthAutoExposure = value;
		return setBoolProperty(OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL, value);
	}

	bool _Orbbec::setDepthExposure(int32_t value)
	{
		m_orbbecCtrl.m_depthExposure = value;
		return setIntProperty(OB_PROP_DEPTH_EXPOSURE_INT, value);
	}

	bool _Orbbec::setDepthGain(int32_t value)
	{
		m_orbbecCtrl.m_depthGain = value;
		return setIntProperty(OB_PROP_DEPTH_GAIN_INT, value);
	}

	bool _Orbbec::setIRAutoExposure(bool value)
	{
		m_orbbecCtrl.m_irAutoExposure = value;
		return setBoolProperty(OB_PROP_IR_AUTO_EXPOSURE_BOOL, value);
	}

	bool _Orbbec::setIRExposure(int32_t value)
	{
		m_orbbecCtrl.m_irExposure = value;
		return setIntProperty(OB_PROP_IR_EXPOSURE_INT, value);
	}

	bool _Orbbec::setIRGain(int32_t value)
	{
		m_orbbecCtrl.m_irGain = value;
		return setIntProperty(OB_PROP_IR_GAIN_INT, value);
	}

	bool _Orbbec::setIRChannelDataSource(int32_t value)
	{
		m_orbbecCtrl.m_irChannelDataSource = value;
		return setIntProperty(OB_PROP_IR_CHANNEL_DATA_SOURCE_INT, value);
	}

	bool _Orbbec::setDepthRMFilter(bool value)
	{
		m_orbbecCtrl.m_depthRMFilter = value;
		return setBoolProperty(OB_PROP_DEPTH_RM_FILTER_BOOL, value);
	}

	bool _Orbbec::setColorAEMaxGain(int32_t value)
	{
		m_orbbecCtrl.m_colorAEMaxGain = value;
		return setIntProperty(OB_PROP_COLOR_AE_MAX_GAIN_INT, value);
	}

	bool _Orbbec::setDepthAutoExposurePriority(int32_t value)
	{
		m_orbbecCtrl.m_depthAutoExposurePriority = value;
		return setIntProperty(OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT, value);
	}

	bool _Orbbec::setSDKDisparityToDepth(bool value)
	{
		m_orbbecCtrl.m_sdkDisparityToDepth = value;
		return setBoolProperty(OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL, value);
	}

	bool _Orbbec::setSDKAccelFrameTransformed(bool value)
	{
		m_orbbecCtrl.m_sdkAccelFrameTransformed = value;
		return setBoolProperty(OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL, value);
	}

	bool _Orbbec::setSDKGyroFrameTransformed(bool value)
	{
		m_orbbecCtrl.m_sdkGyroFrameTransformed = value;
		return setBoolProperty(OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL, value);
	}

	bool _Orbbec::setDevicePerformanceMode(int32_t value)
	{
		m_orbbecCtrl.m_devicePerformanceMode = value;
		return setIntProperty(OB_PROP_DEVICE_PERFORMANCE_MODE_INT, value);
	}

	bool _Orbbec::setColorDenoisingLevel(int32_t value)
	{
		m_orbbecCtrl.m_colorDenoisingLevel = value;
		return setIntProperty(OB_PROP_COLOR_DENOISING_LEVEL_INT, value);
	}

	bool _Orbbec::setMultiDeviceSyncConfig(const OBMultiDeviceSyncConfig &value)
	{
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
		m_orbbecCtrl.m_deviceIPAddrConfig = value;
		return setStructuredData(OB_STRUCT_DEVICE_IP_ADDR_CONFIG, &value, sizeof(value));
	}

	bool _Orbbec::setDepthHdrConfig(const OBHdrConfig &value)
	{
		m_orbbecCtrl.m_depthHdrConfig = value;
		return setStructuredData(OB_STRUCT_DEPTH_HDR_CONFIG, &value, sizeof(value));
	}

	bool _Orbbec::setColorAEROI(const OBRegionOfInterest &value)
	{
		m_orbbecCtrl.m_colorAEROI = value;
		return setStructuredData(OB_STRUCT_COLOR_AE_ROI, &value, sizeof(value));
	}

	bool _Orbbec::setDepthAEROI(const OBRegionOfInterest &value)
	{
		m_orbbecCtrl.m_depthAEROI = value;
		return setStructuredData(OB_STRUCT_DEPTH_AE_ROI, &value, sizeof(value));
	}

	bool _Orbbec::setDispOffsetConfig(const OBDispOffsetConfig &value)
	{
		m_orbbecCtrl.m_dispOffsetConfig = value;
		return setStructuredData(OB_STRUCT_DISP_OFFSET_CONFIG, &value, sizeof(value));
	}

	bool _Orbbec::setPresetResolutionConfig(const OBPresetResolutionConfig &value)
	{
		m_orbbecCtrl.m_presetResolutionConfig = value;
		return setStructuredData(OB_STRUCT_PRESET_RESOLUTION_CONFIG, &value, sizeof(value));
	}

	bool _Orbbec::setDeviceIPAddrConfigV2(const OBNetIpConfigV2 &value)
	{
		m_orbbecCtrl.m_deviceIPAddrConfigV2 = value;
		return setStructuredData(OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2, &value, sizeof(value));
	}

	bool _Orbbec::setTimestampResetConfig(const OBDeviceTimestampResetConfig &value)
	{
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
