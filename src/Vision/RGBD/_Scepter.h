/*
 * _Scepter.h
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__Scepter_H_
#define OpenKAI_src_Vision_RGBD__Scepter_H_

#include "_RGBDbase.h"
#include <Scepter_api.h>
#include <Scepter_Morph_api.h>
#include <map>
#include <vector>

namespace kai
{
	struct ScCtrl
	{
		int m_tScan = 3000; // scan time
		int m_pixelFormat = SC_PIXEL_FORMAT_BGR_888;

		bool m_bAutoExposureToF = true;
		int m_tExposureToF = 4000;

		bool m_bAutoExposureRGB = true;
		int m_tExposureRGB = 4000;

		// SDK parameter order: threshold, enable.
		ScTimeFilterParams m_timeFilter = {1, true}; // [1, 6]
		ScConfidenceFilterParams m_confidenceFilter = {1, true}; // [1, 100]
		ScFlyingPixelFilterParams m_flyingPixelFilter = {5, true}; // [1, 16]

		bool m_bFillHole = false;
		bool m_bSpatialFilter = false;
		bool m_bHDR = false;

		// Application defaults; supported values depend on the camera model.
		int m_frameRate = 30;
		ScWorkMode m_workMode = SC_ACTIVE_MODE;
		uint8_t m_softwareTriggerFrameCount = 1; // [1, 10]
		ScInputSignalParamsForHWTrigger m_hwTrigger = {0, 0, 0};
		ScResolution m_resolutionToF = {640, 480};
		ScResolution m_resolutionRGB = {1280, 720};
		bool m_bTransformRGBToDepth = false;
		bool m_bTransformDepthToRGB = false;

		uint8_t m_irGMMGain = 50;
		ScIRGMMCorrectionParams m_irGMMCorrection = {1, false}; // threshold [1, 100], enable
		float m_gainRGB = 1.0f;
		int m_tAECMaxExposureRGB = 4000; // microseconds
		ScVector2u16 m_aecROIRGBorigin = {0, 0};
		ScResolution m_aecROIRGBsize = {1280, 720};

		bool m_bWDR = true;
		// Only explicitly configured frame indices are stored; counts vary by device.
		std::map<uint8_t, int> m_tExposureHDR = {}; // microseconds
		std::map<uint8_t, int> m_tExposureWDR = {}; // microseconds

		bool m_bDHCP = false;
		string m_deviceIPAddr = "";
		string m_deviceSubnetMask = "";
		ScTimeSyncConfig m_timeSync = {0, {0}}; // 0: off, 1: NTP, 2: PTP
		string m_paramsJsonFile = "";
		PtrHotPlugStatusCallback m_hotPlugCallback = nullptr;
		const void *m_pHotPlugUserData = nullptr; // owned by the caller

		// Morph AI options are supported only by devices with an AI module.
		bool m_bAIModule = false;
		ScAIModuleMode m_aiWorkMode = AI_CONTINUOUS_RUN_MODE;
		std::map<uint32_t, std::vector<uint8_t>> m_aiParams = {};
		std::map<ScFrameType, bool> m_aiInputFrames = {};
		std::map<ScFrameType, bool> m_aiPreviewFrames = {};
	};

	class _Scepter : public _RGBDbase
	{
	public:
		_Scepter();
		virtual ~_Scepter();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

		virtual bool open(void);
		virtual void close(void);

		ScCtrl getCamCtrl(void);
		bool setCamCtrl(const ScCtrl &camCtrl);
		bool setToFexposureControlMode(bool bAuto);
		bool setToFexposureTime(int tExposure);
		bool setRGBexposureControlMode(bool bAuto);
		bool setRGBexposureTime(int tExposure);
		bool setTimeFilter(bool bON, int thr);
		bool setConfidenceFilter(bool bON, int thr);
		bool setFlyingPixelFilter(bool bON, int thr);
		bool setFillHole(bool bON);
		bool setSpatialFilter(bool bON);
		bool setHDR(bool bON);
		bool setFrameRate(int fps);
		bool setWorkMode(ScWorkMode mode);
		bool setSoftwareTriggerParameter(uint8_t frameCount);
		bool setInputSignalParamsForHWTrigger(uint32_t width, uint32_t interval, uint8_t polarity);
		bool setToFResolution(int width, int height);
		bool setColorResolution(int width, int height);
		bool setColorPixelFormat(ScPixelFormat pixelFormat);
		bool setTransformColorImgToDepthSensorEnabled(bool bON);
		bool setTransformDepthImgToColorSensorEnabled(bool bON);
		bool setIRGMMGain(uint8_t gain);
		bool setIRGMMCorrection(bool bON, int thr);
		bool setColorGain(float gain);
		bool setColorAECMaxExposureTime(int tExposure);
		bool setColorAECROI(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
		bool setExposureTimeOfHDR(uint8_t frameIndex, int tExposure);
		bool setWDR(bool bON);
		bool setExposureTimeOfWDR(uint8_t frameIndex, int tExposure);
		bool setDeviceDHCPEnabled(bool bON);
		bool setDeviceIPAddr(const string &ipAddr);
		bool setDeviceSubnetMask(const string &mask);
		bool setRealTimeSyncConfig(const ScTimeSyncConfig &params);
		bool setParamsByJson(const string &filePath);
		bool setHotPlugStatusCallback(PtrHotPlugStatusCallback pCallback, const void *pUserData);
		bool setAIModuleEnabled(bool bON);
		bool setAIModuleWorkMode(ScAIModuleMode mode);
		bool setAIModuleParam(uint32_t paramID, const std::vector<uint8_t> &data);
		bool setAIModuleInputFrameTypeEnabled(ScFrameType frameType, bool bON);
		bool setAIModulePreviewFrameTypeEnabled(ScFrameType frameType, bool bON);

	protected:
		bool updateScRGBD(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Scepter *)This)->update();
			return NULL;
		}

		void updatePCL(void);
		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_Scepter *)This)->updateTPP();
			return NULL;
		}

	protected:
		// SDK frame buffers remain valid only until the next capture. Serialize
		// capture/conversion/close; the PCL worker publishes owned data afterwards.
		std::mutex m_mutexScFrame;
		bool m_bPCLframe = false;
		bool m_bScInitialized = false;
		int m_pclStride = 1; // Sample every Nth depth pixel in both axes; 1 keeps all points.
		uint32_t m_nDevice = 0;
		ScDeviceInfo *m_pScDevListInfo = nullptr;
		ScDeviceHandle m_scDevHandle = 0;
		ScSensorIntrinsicParameters m_scCamParams;
		ScCtrl m_scCtrl;
		uint64_t m_tFrameInterval = 0; // minimal interval between frame reading

		ScFrame m_scfRGB = {0};
		ScFrame m_scfDepth = {0};
		ScFrame m_scfTransformedDepth = {0};
		ScFrame m_scfTransformedRGB = {0};
		ScFrame m_scfIR = {0};

	};

}
#endif
