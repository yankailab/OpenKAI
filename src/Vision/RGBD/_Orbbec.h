/*
 * _Orbbec.h
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__Orbbec_H_
#define OpenKAI_src_Vision_RGBD__Orbbec_H_

#include <libobsensor/ObSensor.hpp>
#include <libobsensor/h/ObTypes.h>
#include "_RGBDbase.h"
#include <optional>
#include <vector>

namespace kai
{
	struct OrbbecCtrl
	{
		// Gemini device controls from OrbbecSDK_v2 Property.h; JSON keys are the SDK IDs.
		// nullopt (or JSON null) preserves the device setting; no property is written
		// unless configured. Availability and valid ranges depend on the Gemini model.
		// Read-only status and one-shot commands are not configuration options.
		std::optional<bool> m_ldp = std::nullopt; // OB_PROP_LDP_BOOL
		std::optional<bool> m_laser = std::nullopt; // OB_PROP_LASER_BOOL
		std::optional<bool> m_flood = std::nullopt; // OB_PROP_FLOOD_BOOL
		std::optional<bool> m_temperatureCompensation = std::nullopt; // OB_PROP_TEMPERATURE_COMPENSATION_BOOL
		std::optional<bool> m_depthMirror = std::nullopt; // OB_PROP_DEPTH_MIRROR_BOOL
		std::optional<bool> m_depthFlip = std::nullopt; // OB_PROP_DEPTH_FLIP_BOOL
		std::optional<bool> m_depthHolefilter = std::nullopt; // OB_PROP_DEPTH_HOLEFILTER_BOOL
		std::optional<bool> m_irMirror = std::nullopt; // OB_PROP_IR_MIRROR_BOOL
		std::optional<bool> m_irFlip = std::nullopt; // OB_PROP_IR_FLIP_BOOL
		std::optional<bool> m_depthNoiseRemovalFilter = std::nullopt; // OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_BOOL
		std::optional<int32_t> m_depthNoiseRemovalFilterMaxDiff = std::nullopt; // OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_DIFF_INT
		std::optional<int32_t> m_depthNoiseRemovalFilterMaxSpeckleSize = std::nullopt; // OB_PROP_DEPTH_NOISE_REMOVAL_FILTER_MAX_SPECKLE_SIZE_INT
		std::optional<bool> m_depthAlignHardware = std::nullopt; // OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL
		std::optional<int32_t> m_depthAlignHardwareMode = std::nullopt; // OB_PROP_DEPTH_ALIGN_HARDWARE_MODE_INT
		std::optional<int32_t> m_depthPrecisionLevel = std::nullopt; // OB_PROP_DEPTH_PRECISION_LEVEL_INT
		std::optional<bool> m_colorMirror = std::nullopt; // OB_PROP_COLOR_MIRROR_BOOL
		std::optional<bool> m_colorFlip = std::nullopt; // OB_PROP_COLOR_FLIP_BOOL
		std::optional<bool> m_disparityToDepth = std::nullopt; // OB_PROP_DISPARITY_TO_DEPTH_BOOL
		std::optional<bool> m_watchdog = std::nullopt; // OB_PROP_WATCHDOG_BOOL
		std::optional<bool> m_externalSignalReset = std::nullopt; // OB_PROP_EXTERNAL_SIGNAL_RESET_BOOL
		std::optional<bool> m_heartbeat = std::nullopt; // OB_PROP_HEARTBEAT_BOOL
		std::optional<int32_t> m_laserPowerLevelControl = std::nullopt; // OB_PROP_LASER_POWER_LEVEL_CONTROL_INT
		std::optional<bool> m_timerResetTriggerOutEnable = std::nullopt; // OB_PROP_TIMER_RESET_TRIGGER_OUT_ENABLE_BOOL
		std::optional<int32_t> m_timerResetDelayUs = std::nullopt; // OB_PROP_TIMER_RESET_DELAY_US_INT
		std::optional<bool> m_irRightMirror = std::nullopt; // OB_PROP_IR_RIGHT_MIRROR_BOOL
		std::optional<int32_t> m_captureImageFrameNumber = std::nullopt; // OB_PROP_CAPTURE_IMAGE_FRAME_NUMBER_INT
		std::optional<bool> m_irRightFlip = std::nullopt; // OB_PROP_IR_RIGHT_FLIP_BOOL
		std::optional<int32_t> m_colorRotate = std::nullopt; // OB_PROP_COLOR_ROTATE_INT
		std::optional<int32_t> m_irRotate = std::nullopt; // OB_PROP_IR_ROTATE_INT
		std::optional<int32_t> m_irRightRotate = std::nullopt; // OB_PROP_IR_RIGHT_ROTATE_INT
		std::optional<int32_t> m_depthRotate = std::nullopt; // OB_PROP_DEPTH_ROTATE_INT
		std::optional<bool> m_syncSignalTriggerOut = std::nullopt; // OB_PROP_SYNC_SIGNAL_TRIGGER_OUT_BOOL
		std::optional<bool> m_deviceUSB2RepeatIdentify = std::nullopt; // OB_PROP_DEVICE_USB2_REPEAT_IDENTIFY_BOOL
		std::optional<bool> m_laserAlwaysOn = std::nullopt; // OB_PROP_LASER_ALWAYS_ON_BOOL
		std::optional<int32_t> m_laserOnOffPattern = std::nullopt; // OB_PROP_LASER_ON_OFF_PATTERN_INT
		std::optional<float> m_depthUnitFlexibleAdjustment = std::nullopt; // OB_PROP_DEPTH_UNIT_FLEXIBLE_ADJUSTMENT_FLOAT
		std::optional<int32_t> m_laserControl = std::nullopt; // OB_PROP_LASER_CONTROL_INT
		std::optional<int32_t> m_irBrightness = std::nullopt; // OB_PROP_IR_BRIGHTNESS_INT
		std::optional<int32_t> m_colorAEMaxExposure = std::nullopt; // OB_PROP_COLOR_AE_MAX_EXPOSURE_INT
		std::optional<int32_t> m_irAEMaxExposure = std::nullopt; // OB_PROP_IR_AE_MAX_EXPOSURE_INT
		std::optional<int32_t> m_dispSearchRangeMode = std::nullopt; // OB_PROP_DISP_SEARCH_RANGE_MODE_INT
		std::optional<int32_t> m_dispSearchOffset = std::nullopt; // OB_PROP_DISP_SEARCH_OFFSET_INT
		std::optional<bool> m_cpuTemperatureCalibration = std::nullopt; // OB_PROP_CPU_TEMPERATURE_CALIBRATION_BOOL
		std::optional<int32_t> m_frameInterleaveConfigIndex = std::nullopt; // OB_PROP_FRAME_INTERLEAVE_CONFIG_INDEX_INT
		std::optional<bool> m_frameInterleaveEnable = std::nullopt; // OB_PROP_FRAME_INTERLEAVE_ENABLE_BOOL
		std::optional<int32_t> m_frameInterleaveLaserPatternSyncDelay = std::nullopt; // OB_PROP_FRAME_INTERLEAVE_LASER_PATTERN_SYNC_DELAY_INT
		std::optional<bool> m_onChipCalibrationEnable = std::nullopt; // OB_PROP_ON_CHIP_CALIBRATION_ENABLE_BOOL
		std::optional<bool> m_hwNoiseRemoveFilterEnable = std::nullopt; // OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL
		std::optional<float> m_hwNoiseRemoveFilterThreshold = std::nullopt; // OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT
		std::optional<bool> m_deviceAutoCaptureEnable = std::nullopt; // OB_DEVICE_AUTO_CAPTURE_ENABLE_BOOL
		std::optional<int32_t> m_deviceAutoCaptureIntervalTime = std::nullopt; // OB_DEVICE_AUTO_CAPTURE_INTERVAL_TIME_INT
		std::optional<bool> m_devicePTPClockSyncEnable = std::nullopt; // OB_DEVICE_PTP_CLOCK_SYNC_ENABLE_BOOL
		std::optional<bool> m_confidenceStreamFilter = std::nullopt; // OB_PROP_CONFIDENCE_STREAM_FILTER_BOOL
		std::optional<int32_t> m_confidenceStreamFilterThreshold = std::nullopt; // OB_PROP_CONFIDENCE_STREAM_FILTER_THRESHOLD_INT
		std::optional<bool> m_confidenceMirror = std::nullopt; // OB_PROP_CONFIDENCE_MIRROR_BOOL
		std::optional<bool> m_confidenceFlip = std::nullopt; // OB_PROP_CONFIDENCE_FLIP_BOOL
		std::optional<int32_t> m_confidenceRotate = std::nullopt; // OB_PROP_CONFIDENCE_ROTATE_INT
		std::optional<int32_t> m_intraCameraSyncReference = std::nullopt; // OB_PROP_INTRA_CAMERA_SYNC_REFERENCE_INT
		std::optional<int32_t> m_colorRightRotate = std::nullopt; // OB_PROP_COLOR_RIGHT_ROTATE_INT
		std::optional<bool> m_colorRightMirror = std::nullopt; // OB_PROP_COLOR_RIGHT_MIRROR_BOOL
		std::optional<bool> m_colorRightFlip = std::nullopt; // OB_PROP_COLOR_RIGHT_FLIP_BOOL
		std::optional<int32_t> m_deviceAEReference = std::nullopt; // OB_PROP_DEVICE_AE_REFERENCE_INT
		std::optional<int32_t> m_deviceAEStrategy = std::nullopt; // OB_PROP_DEVICE_AE_STRATEGY_INT
		std::optional<int32_t> m_colorROIBrightness = std::nullopt; // OB_PROP_COLOR_ROI_BRIGHTNESS_INT
		std::optional<int32_t> m_colorLeftRotate = std::nullopt; // OB_PROP_COLOR_LEFT_ROTATE_INT
		std::optional<bool> m_colorLeftMirror = std::nullopt; // OB_PROP_COLOR_LEFT_MIRROR_BOOL
		std::optional<bool> m_colorLeftFlip = std::nullopt; // OB_PROP_COLOR_LEFT_FLIP_BOOL
		std::optional<int32_t> m_colorPresetPriority = std::nullopt; // OB_PROP_COLOR_PRESET_PRIORITY_INT
		std::optional<bool> m_deviceNetworkLLA = std::nullopt; // OB_PROP_DEVICE_NETWORK_LLA_BOOL
		std::optional<bool> m_colorAntiFlicker = std::nullopt; // OB_PROP_COLOR_ANTI_FLICKER_BOOL
		std::optional<int32_t> m_deviceIPMode = std::nullopt; // OB_PROP_DEVICE_IP_MODE_INT
		std::optional<int32_t> m_dhcpAssignIPTimeout = std::nullopt; // OB_PROP_DHCP_ASSIGN_IP_TIMEOUT_INT
		std::optional<int32_t> m_usbSyncVoltageLevel = std::nullopt; // OB_PROP_USB_SYNC_VOLTAGE_LEVEL_INT
		std::optional<bool> m_fpsBoost = std::nullopt; // OB_PROP_FPS_BOOST_BOOL
		std::optional<int32_t> m_mjpegQuality = std::nullopt; // OB_PROP_MJPEG_QUALITY_INT
		std::optional<bool> m_colorAutoExposure = std::nullopt; // OB_PROP_COLOR_AUTO_EXPOSURE_BOOL
		std::optional<int32_t> m_colorExposure = std::nullopt; // OB_PROP_COLOR_EXPOSURE_INT
		std::optional<int32_t> m_colorGain = std::nullopt; // OB_PROP_COLOR_GAIN_INT
		std::optional<bool> m_colorAutoWhiteBalance = std::nullopt; // OB_PROP_COLOR_AUTO_WHITE_BALANCE_BOOL
		std::optional<int32_t> m_colorWhiteBalance = std::nullopt; // OB_PROP_COLOR_WHITE_BALANCE_INT
		std::optional<int32_t> m_colorBrightness = std::nullopt; // OB_PROP_COLOR_BRIGHTNESS_INT
		std::optional<int32_t> m_colorSharpness = std::nullopt; // OB_PROP_COLOR_SHARPNESS_INT
		std::optional<int32_t> m_colorSaturation = std::nullopt; // OB_PROP_COLOR_SATURATION_INT
		std::optional<int32_t> m_colorContrast = std::nullopt; // OB_PROP_COLOR_CONTRAST_INT
		std::optional<int32_t> m_colorGamma = std::nullopt; // OB_PROP_COLOR_GAMMA_INT
		std::optional<int32_t> m_colorAutoExposurePriority = std::nullopt; // OB_PROP_COLOR_AUTO_EXPOSURE_PRIORITY_INT
		std::optional<int32_t> m_colorBacklightCompensation = std::nullopt; // OB_PROP_COLOR_BACKLIGHT_COMPENSATION_INT
		std::optional<int32_t> m_colorHue = std::nullopt; // OB_PROP_COLOR_HUE_INT
		std::optional<int32_t> m_colorPowerLineFrequency = std::nullopt; // OB_PROP_COLOR_POWER_LINE_FREQUENCY_INT
		std::optional<bool> m_depthAutoExposure = std::nullopt; // OB_PROP_DEPTH_AUTO_EXPOSURE_BOOL
		std::optional<int32_t> m_depthExposure = std::nullopt; // OB_PROP_DEPTH_EXPOSURE_INT
		std::optional<int32_t> m_depthGain = std::nullopt; // OB_PROP_DEPTH_GAIN_INT
		std::optional<bool> m_irAutoExposure = std::nullopt; // OB_PROP_IR_AUTO_EXPOSURE_BOOL
		std::optional<int32_t> m_irExposure = std::nullopt; // OB_PROP_IR_EXPOSURE_INT
		std::optional<int32_t> m_irGain = std::nullopt; // OB_PROP_IR_GAIN_INT
		std::optional<int32_t> m_irChannelDataSource = std::nullopt; // OB_PROP_IR_CHANNEL_DATA_SOURCE_INT
		std::optional<bool> m_depthRMFilter = std::nullopt; // OB_PROP_DEPTH_RM_FILTER_BOOL
		std::optional<int32_t> m_colorAEMaxGain = std::nullopt; // OB_PROP_COLOR_AE_MAX_GAIN_INT
		std::optional<int32_t> m_depthAutoExposurePriority = std::nullopt; // OB_PROP_DEPTH_AUTO_EXPOSURE_PRIORITY_INT
		std::optional<bool> m_sdkDisparityToDepth = std::nullopt; // OB_PROP_SDK_DISPARITY_TO_DEPTH_BOOL
		std::optional<bool> m_sdkAccelFrameTransformed = std::nullopt; // OB_PROP_SDK_ACCEL_FRAME_TRANSFORMED_BOOL
		std::optional<bool> m_sdkGyroFrameTransformed = std::nullopt; // OB_PROP_SDK_GYRO_FRAME_TRANSFORMED_BOOL
		std::optional<int32_t> m_devicePerformanceMode = std::nullopt; // OB_PROP_DEVICE_PERFORMANCE_MODE_INT
		std::optional<int32_t> m_colorDenoisingLevel = std::nullopt; // OB_PROP_COLOR_DENOISING_LEVEL_INT

		// Structured JSON values use the SDK field names; supply every listed field.
		std::optional<OBMultiDeviceSyncConfig> m_multiDeviceSyncConfig = std::nullopt; // OB_STRUCT_MULTI_DEVICE_SYNC_CONFIG
		std::optional<OBNetIpConfig> m_deviceIPAddrConfig = std::nullopt; // OB_STRUCT_DEVICE_IP_ADDR_CONFIG
		std::optional<OBHdrConfig> m_depthHdrConfig = std::nullopt; // OB_STRUCT_DEPTH_HDR_CONFIG
		std::optional<OBRegionOfInterest> m_colorAEROI = std::nullopt; // OB_STRUCT_COLOR_AE_ROI
		std::optional<OBRegionOfInterest> m_depthAEROI = std::nullopt; // OB_STRUCT_DEPTH_AE_ROI
		std::optional<OBDispOffsetConfig> m_dispOffsetConfig = std::nullopt; // OB_STRUCT_DISP_OFFSET_CONFIG
		std::optional<OBPresetResolutionConfig> m_presetResolutionConfig = std::nullopt; // OB_STRUCT_PRESET_RESOLUTION_CONFIG
		std::optional<OBNetIpConfigV2> m_deviceIPAddrConfigV2 = std::nullopt; // OB_STRUCT_DEVICE_IP_ADDR_CONFIG_V2
		std::optional<OBDeviceTimestampResetConfig> m_timestampResetConfig = std::nullopt; // obTimestampResetConfig

		// Advanced device APIs; presets are loaded before individual properties.
		std::optional<string> m_depthWorkMode = std::nullopt; // obDepthWorkMode
		std::optional<string> m_colorPreset = std::nullopt; // obColorPreset
		std::optional<string> m_preset = std::nullopt; // obPreset
		std::optional<string> m_presetJsonFile = std::nullopt; // obPresetJsonFile
		std::optional<string> m_frameInterleave = std::nullopt; // obFrameInterleave
		std::optional<bool> m_globalTimestamp = std::nullopt; // obGlobalTimestamp
		std::optional<bool> m_firmwareLog = std::nullopt; // obFirmwareLog
	};

	class _Orbbec : public _RGBDbase
	{
	public:
		_Orbbec();
		virtual ~_Orbbec();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual bool open(void);
		virtual void close(void);

		OrbbecCtrl getCamCtrl(void) const;
		bool setCamCtrl(const OrbbecCtrl &camCtrl);
		// Setters cache the requested value even when no device is open.
		bool setLDP(bool value);
		bool setLaser(bool value);
		bool setFlood(bool value);
		bool setTemperatureCompensation(bool value);
		bool setDepthMirror(bool value);
		bool setDepthFlip(bool value);
		bool setDepthHolefilter(bool value);
		bool setIRMirror(bool value);
		bool setIRFlip(bool value);
		bool setDepthNoiseRemovalFilter(bool value);
		bool setDepthNoiseRemovalFilterMaxDiff(int32_t value);
		bool setDepthNoiseRemovalFilterMaxSpeckleSize(int32_t value);
		bool setDepthAlignHardware(bool value);
		bool setDepthAlignHardwareMode(int32_t value);
		bool setDepthPrecisionLevel(int32_t value);
		bool setColorMirror(bool value);
		bool setColorFlip(bool value);
		bool setDisparityToDepth(bool value);
		bool setWatchdog(bool value);
		bool setExternalSignalReset(bool value);
		bool setHeartbeat(bool value);
		bool setLaserPowerLevelControl(int32_t value);
		bool setTimerResetTriggerOutEnable(bool value);
		bool setTimerResetDelayUs(int32_t value);
		bool setIRRightMirror(bool value);
		bool setCaptureImageFrameNumber(int32_t value);
		bool setIRRightFlip(bool value);
		bool setColorRotate(int32_t value);
		bool setIRRotate(int32_t value);
		bool setIRRightRotate(int32_t value);
		bool setDepthRotate(int32_t value);
		bool setSyncSignalTriggerOut(bool value);
		bool setDeviceUSB2RepeatIdentify(bool value);
		bool setLaserAlwaysOn(bool value);
		bool setLaserOnOffPattern(int32_t value);
		bool setDepthUnitFlexibleAdjustment(float value);
		bool setLaserControl(int32_t value);
		bool setIRBrightness(int32_t value);
		bool setColorAEMaxExposure(int32_t value);
		bool setIRAEMaxExposure(int32_t value);
		bool setDispSearchRangeMode(int32_t value);
		bool setDispSearchOffset(int32_t value);
		bool setCPUTemperatureCalibration(bool value);
		bool setFrameInterleaveConfigIndex(int32_t value);
		bool setFrameInterleaveEnable(bool value);
		bool setFrameInterleaveLaserPatternSyncDelay(int32_t value);
		bool setOnChipCalibrationEnable(bool value);
		bool setHWNoiseRemoveFilterEnable(bool value);
		bool setHWNoiseRemoveFilterThreshold(float value);
		bool setDeviceAutoCaptureEnable(bool value);
		bool setDeviceAutoCaptureIntervalTime(int32_t value);
		bool setDevicePTPClockSyncEnable(bool value);
		bool setConfidenceStreamFilter(bool value);
		bool setConfidenceStreamFilterThreshold(int32_t value);
		bool setConfidenceMirror(bool value);
		bool setConfidenceFlip(bool value);
		bool setConfidenceRotate(int32_t value);
		bool setIntraCameraSyncReference(int32_t value);
		bool setColorRightRotate(int32_t value);
		bool setColorRightMirror(bool value);
		bool setColorRightFlip(bool value);
		bool setDeviceAEReference(int32_t value);
		bool setDeviceAEStrategy(int32_t value);
		bool setColorROIBrightness(int32_t value);
		bool setColorLeftRotate(int32_t value);
		bool setColorLeftMirror(bool value);
		bool setColorLeftFlip(bool value);
		bool setColorPresetPriority(int32_t value);
		bool setDeviceNetworkLLA(bool value);
		bool setColorAntiFlicker(bool value);
		bool setDeviceIPMode(int32_t value);
		bool setDHCPAssignIPTimeout(int32_t value);
		bool setUSBSyncVoltageLevel(int32_t value);
		bool setFPSBoost(bool value);
		bool setMJPEGQuality(int32_t value);
		bool setColorAutoExposure(bool value);
		bool setColorExposure(int32_t value);
		bool setColorGain(int32_t value);
		bool setColorAutoWhiteBalance(bool value);
		bool setColorWhiteBalance(int32_t value);
		bool setColorBrightness(int32_t value);
		bool setColorSharpness(int32_t value);
		bool setColorSaturation(int32_t value);
		bool setColorContrast(int32_t value);
		bool setColorGamma(int32_t value);
		bool setColorAutoExposurePriority(int32_t value);
		bool setColorBacklightCompensation(int32_t value);
		bool setColorHue(int32_t value);
		bool setColorPowerLineFrequency(int32_t value);
		bool setDepthAutoExposure(bool value);
		bool setDepthExposure(int32_t value);
		bool setDepthGain(int32_t value);
		bool setIRAutoExposure(bool value);
		bool setIRExposure(int32_t value);
		bool setIRGain(int32_t value);
		bool setIRChannelDataSource(int32_t value);
		bool setDepthRMFilter(bool value);
		bool setColorAEMaxGain(int32_t value);
		bool setDepthAutoExposurePriority(int32_t value);
		bool setSDKDisparityToDepth(bool value);
		bool setSDKAccelFrameTransformed(bool value);
		bool setSDKGyroFrameTransformed(bool value);
		bool setDevicePerformanceMode(int32_t value);
		bool setColorDenoisingLevel(int32_t value);
		bool setMultiDeviceSyncConfig(const OBMultiDeviceSyncConfig &value);
		bool setDeviceIPAddrConfig(const OBNetIpConfig &value);
		bool setDepthHdrConfig(const OBHdrConfig &value);
		bool setColorAEROI(const OBRegionOfInterest &value);
		bool setDepthAEROI(const OBRegionOfInterest &value);
		bool setDispOffsetConfig(const OBDispOffsetConfig &value);
		bool setPresetResolutionConfig(const OBPresetResolutionConfig &value);
		bool setDeviceIPAddrConfigV2(const OBNetIpConfigV2 &value);
		bool setTimestampResetConfig(const OBDeviceTimestampResetConfig &value);
		bool switchDepthWorkMode(const string &value);
		bool switchColorPreset(const string &value);
		bool loadPreset(const string &value);
		bool loadPresetFromJsonFile(const string &value);
		bool loadFrameInterleave(const string &value);
		bool enableGlobalTimestamp(bool value);
		bool enableFirmwareLog(bool value);

	private:
		bool setBoolProperty(OBPropertyID propertyId, bool value);
		bool setIntProperty(OBPropertyID propertyId, int32_t value);
		bool setFloatProperty(OBPropertyID propertyId, float value);
		bool setStructuredData(OBPropertyID propertyId, const void *data, uint32_t size);

		bool updateOrbbec(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Orbbec *)This)->update();
			return NULL;
		}

		void updatePCL(void);
		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_Orbbec *)This)->updateTPP();
			return NULL;
		}

		uint64_t frameTsUs_(const shared_ptr<ob::Frame> &f)
		{
			// Prefer "system timestamp us" if available in your SDK build.
			// If not, fall back to other timestamp APIs.
			return f->getSystemTimeStampUs();
		}

	protected:
		string m_SN = "";
		ob::Context m_ctx;
		shared_ptr<ob::Device> m_spDev = nullptr;
		shared_ptr<ob::Pipeline> m_spPipe = nullptr;
		shared_ptr<ob::SensorList> m_spSensorList = nullptr;
		shared_ptr<ob::Config> m_spConfig = nullptr;
		uint32_t m_tOutMs = 100;

		shared_ptr<ob::PointCloudFilter> m_spPCF = nullptr;
		shared_ptr<ob::Frame> m_spFrame = nullptr;

		uint64_t m_tDus;
		uint64_t m_dtDus;
		uint64_t m_tRGBus;
		uint64_t m_dtRGBus;

		OrbbecCtrl m_orbbecCtrl;
	};

}
#endif
