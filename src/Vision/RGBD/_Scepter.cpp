/*
 * _Scepter.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_Scepter.h"

namespace kai
{

	_Scepter::_Scepter()
	{
	}

	_Scepter::~_Scepter()
	{
	}

	bool _Scepter::init(const json &j)
	{
		IF_F(!_RGBDbase::init(j));

		jKv(j, "scScanTime", m_scCtrl.m_tScan);
		jKv(j, "scPixelFormat", m_scCtrl.m_pixelFormat);
		jKv(j, "scAutoExposureToF", m_scCtrl.m_bAutoExposureToF);
		jKv(j, "scExposureTimeToF", m_scCtrl.m_tExposureToF);
		jKv(j, "scAutoExposureRGB", m_scCtrl.m_bAutoExposureRGB);
		jKv(j, "scExposureTimeRGB", m_scCtrl.m_tExposureRGB);
		jKv(j, "scTimeFilterEnabled", m_scCtrl.m_timeFilter.enable);
		jKv(j, "scTimeFilterThreshold", m_scCtrl.m_timeFilter.threshold);
		jKv(j, "scConfidenceFilterEnabled", m_scCtrl.m_confidenceFilter.enable);
		jKv(j, "scConfidenceFilterThreshold", m_scCtrl.m_confidenceFilter.threshold);
		jKv(j, "scFlyingPixelFilterEnabled", m_scCtrl.m_flyingPixelFilter.enable);
		jKv(j, "scFlyingPixelFilterThreshold", m_scCtrl.m_flyingPixelFilter.threshold);
		jKv(j, "scFillHoleEnabled", m_scCtrl.m_bFillHole);
		jKv(j, "scSpatialFilterEnabled", m_scCtrl.m_bSpatialFilter);
		jKv(j, "scHDR", m_scCtrl.m_bHDR);

		// Keep existing configurations working; explicit sc options take precedence.
		jKv(jK(j, "thread"), "FPS", m_scCtrl.m_frameRate);
		jKv(j, "scFrameRate", m_scCtrl.m_frameRate);
		IF_Le_F(m_scCtrl.m_frameRate <= 0, "scFrameRate must be positive");

		jKv(j, "scWorkMode", m_scCtrl.m_workMode);
		jKv(j, "scSoftwareTriggerFrameCount", m_scCtrl.m_softwareTriggerFrameCount);
		jKv(j, "scHWTriggerWidth", m_scCtrl.m_hwTrigger.width);
		jKv(j, "scHWTriggerInterval", m_scCtrl.m_hwTrigger.interval);
		jKv(j, "scHWTriggerPolarity", m_scCtrl.m_hwTrigger.polarity);

		m_vSizeD = Vector2i(m_scCtrl.m_resolutionToF.width, m_scCtrl.m_resolutionToF.height);
		jKv<int>(j, "vSizeD", m_vSizeD);
		m_scCtrl.m_resolutionToF = {(uint16_t)m_vSizeD.x(), (uint16_t)m_vSizeD.y()};
		m_scCtrl.m_resolutionRGB = {(uint16_t)m_vSizeRGB.x(), (uint16_t)m_vSizeRGB.y()};

		jKv(j, "scToFWidth", m_scCtrl.m_resolutionToF.width);
		jKv(j, "scToFHeight", m_scCtrl.m_resolutionToF.height);
		jKv(j, "scColorWidth", m_scCtrl.m_resolutionRGB.width);
		jKv(j, "scColorHeight", m_scCtrl.m_resolutionRGB.height);

		m_scCtrl.m_bTransformRGBToDepth = m_btRGB;
		m_scCtrl.m_bTransformDepthToRGB = m_btDepth;
		jKv(j, "scTransformColorToDepth", m_scCtrl.m_bTransformRGBToDepth);
		jKv(j, "scTransformDepthToColor", m_scCtrl.m_bTransformDepthToRGB);

		jKv(j, "scIRGMMGain", m_scCtrl.m_irGMMGain);
		jKv(j, "scIRGMMCorrectionEnabled", m_scCtrl.m_irGMMCorrection.enable);
		jKv(j, "scIRGMMCorrectionThreshold", m_scCtrl.m_irGMMCorrection.threshold);
		jKv(j, "scColorGain", m_scCtrl.m_gainRGB);
		jKv(j, "scColorAECMaxExposureTime", m_scCtrl.m_tAECMaxExposureRGB);
		jKv(j, "scColorAECROIX", m_scCtrl.m_aecROIRGBorigin.x);
		jKv(j, "scColorAECROIY", m_scCtrl.m_aecROIRGBorigin.y);

		m_scCtrl.m_aecROIRGBsize = m_scCtrl.m_resolutionRGB;
		jKv(j, "scColorAECROIWidth", m_scCtrl.m_aecROIRGBsize.width);
		jKv(j, "scColorAECROIHeight", m_scCtrl.m_aecROIRGBsize.height);
		jKv(j, "scWDR", m_scCtrl.m_bWDR);

		// Integer-keyed maps use JSON pairs, e.g. [[0, 4000], [1, 1000]].
		jKv(j, "scHDRExposureTimes", m_scCtrl.m_tExposureHDR);
		jKv(j, "scWDRExposureTimes", m_scCtrl.m_tExposureWDR);

		jKv(j, "scDHCPEnabled", m_scCtrl.m_bDHCP);
		jKv(j, "scDeviceIPAddr", m_scCtrl.m_deviceIPAddr);
		jKv(j, "scDeviceSubnetMask", m_scCtrl.m_deviceSubnetMask);
		jKv(j, "scTimeSyncMode", m_scCtrl.m_timeSync.flag);
		string ntpIP;
		if (jKv(j, "scNTPServerIP", ntpIP))
		{
			IF_Le_F(ntpIP.size() >= sizeof(m_scCtrl.m_timeSync.ip), "scNTPServerIP is too long");
			memset(m_scCtrl.m_timeSync.ip, 0, sizeof(m_scCtrl.m_timeSync.ip));
			memcpy(m_scCtrl.m_timeSync.ip, ntpIP.c_str(), ntpIP.size());
		}
		jKv(j, "scParamsJsonFile", m_scCtrl.m_paramsJsonFile);
		// Hot-plug callback and user-data pointers must be supplied from C++.

		jKv(j, "scAIModuleEnabled", m_scCtrl.m_bAIModule);
		jKv(j, "scAIModuleWorkMode", m_scCtrl.m_aiWorkMode);
		jKv(j, "scAIModuleParams", m_scCtrl.m_aiParams);
		jKv(j, "scAIModuleInputFrames", m_scCtrl.m_aiInputFrames);
		jKv(j, "scAIModulePreviewFrames", m_scCtrl.m_aiPreviewFrames);

		DEL(m_pTpp);
		m_pTpp = createThread(jK(j, "threadPP"), "threadPP");
		NULL_F(m_pTpp);

		return true;
	}

	bool _Scepter::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_RGBDbase::link(j, pM));

		return true;
	}

	bool _Scepter::open(void)
	{
		IF__(m_bOpened, true);

		if (m_scCtrl.m_hotPlugCallback)
			setHotPlugStatusCallback(m_scCtrl.m_hotPlugCallback, m_scCtrl.m_pHotPlugUserData);

		uint32_t m_nDevice = 0;
		ScStatus status = scGetDeviceCount(&m_nDevice, m_scCtrl.m_tScan);
		if (status != ScStatus::SC_OK)
		{
			LOG_E("ScGetDeviceCount failed");
			return false;
		}

		LOG_I("Get device count: " + i2str(m_nDevice));
		IF_F(m_nDevice == 0);

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

		// Load an optional SDK preset first, then apply the configured controls.
		if (!m_scCtrl.m_paramsJsonFile.empty())
			setParamsByJson(m_scCtrl.m_paramsJsonFile);

		setFrameRate(m_scCtrl.m_frameRate);
		setWorkMode(m_scCtrl.m_workMode);
		if (m_scCtrl.m_workMode == SC_SOFTWARE_TRIGGER_MODE)
			setSoftwareTriggerParameter(m_scCtrl.m_softwareTriggerFrameCount);
		if (m_scCtrl.m_workMode == SC_HARDWARE_TRIGGER_MODE)
			setInputSignalParamsForHWTrigger(m_scCtrl.m_hwTrigger.width,
											 m_scCtrl.m_hwTrigger.interval,
											 m_scCtrl.m_hwTrigger.polarity);

		setToFResolution(m_scCtrl.m_resolutionToF.width, m_scCtrl.m_resolutionToF.height);
		setColorResolution(m_scCtrl.m_resolutionRGB.width, m_scCtrl.m_resolutionRGB.height);
		setColorPixelFormat((ScPixelFormat)m_scCtrl.m_pixelFormat);
		setTransformColorImgToDepthSensorEnabled(m_scCtrl.m_bTransformRGBToDepth);
		setTransformDepthImgToColorSensorEnabled(m_scCtrl.m_bTransformDepthToRGB);

		// HDR requires manual ToF exposure; disable HDR before selecting auto exposure.
		if (!m_scCtrl.m_bHDR)
			setHDR(false);
		setToFexposureControlMode(m_scCtrl.m_bAutoExposureToF && !m_scCtrl.m_bHDR);
		if (!m_scCtrl.m_bAutoExposureToF)
			setToFexposureTime(m_scCtrl.m_tExposureToF);
		if (m_scCtrl.m_bHDR)
		{
			setHDR(true);
			for (const auto &exposure : m_scCtrl.m_tExposureHDR)
				setExposureTimeOfHDR(exposure.first, exposure.second);
		}
		setWDR(m_scCtrl.m_bWDR);
		if (m_scCtrl.m_bWDR)
		{
			for (const auto &exposure : m_scCtrl.m_tExposureWDR)
				setExposureTimeOfWDR(exposure.first, exposure.second);
		}

		setRGBexposureControlMode(m_scCtrl.m_bAutoExposureRGB);
		if (m_scCtrl.m_bAutoExposureRGB)
		{
			setColorAECMaxExposureTime(m_scCtrl.m_tAECMaxExposureRGB);
			setColorAECROI(m_scCtrl.m_aecROIRGBorigin.x, m_scCtrl.m_aecROIRGBorigin.y,
						   m_scCtrl.m_aecROIRGBsize.width, m_scCtrl.m_aecROIRGBsize.height);
		}
		else
		{
			setRGBexposureTime(m_scCtrl.m_tExposureRGB);
			setColorGain(m_scCtrl.m_gainRGB);
		}

		setIRGMMGain(m_scCtrl.m_irGMMGain);
		setIRGMMCorrection(m_scCtrl.m_irGMMCorrection.enable, m_scCtrl.m_irGMMCorrection.threshold);
		setTimeFilter(m_scCtrl.m_timeFilter.enable, m_scCtrl.m_timeFilter.threshold);
		setConfidenceFilter(m_scCtrl.m_confidenceFilter.enable, m_scCtrl.m_confidenceFilter.threshold);
		setFlyingPixelFilter(m_scCtrl.m_flyingPixelFilter.enable, m_scCtrl.m_flyingPixelFilter.threshold);
		setFillHole(m_scCtrl.m_bFillHole);
		setSpatialFilter(m_scCtrl.m_bSpatialFilter);

		setDeviceDHCPEnabled(m_scCtrl.m_bDHCP);
		if (!m_scCtrl.m_bDHCP)
		{
			if (!m_scCtrl.m_deviceIPAddr.empty())
				setDeviceIPAddr(m_scCtrl.m_deviceIPAddr);
			if (!m_scCtrl.m_deviceSubnetMask.empty())
				setDeviceSubnetMask(m_scCtrl.m_deviceSubnetMask);
		}
		setRealTimeSyncConfig(m_scCtrl.m_timeSync);

		if (m_scCtrl.m_bAIModule)
		{
			setAIModuleWorkMode(m_scCtrl.m_aiWorkMode);
			for (const auto &param : m_scCtrl.m_aiParams)
				setAIModuleParam(param.first, param.second);
			for (const auto &frame : m_scCtrl.m_aiInputFrames)
				setAIModuleInputFrameTypeEnabled(frame.first, frame.second);
			for (const auto &frame : m_scCtrl.m_aiPreviewFrames)
				setAIModulePreviewFrameTypeEnabled(frame.first, frame.second);
		}
		setAIModuleEnabled(m_scCtrl.m_bAIModule);

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

		int width = m_vSizeD.x();
		int height = m_vSizeD.y();
		if (scGetToFResolution(m_scDevHandle, &width, &height) == SC_OK)
			m_vSizeD = Vector2i(width, height);
		m_pScVw = new ScVector3f[m_vSizeD.x() * m_vSizeD.y()];

		m_tFrameInterval = 2 * 1000 / m_scCtrl.m_frameRate;
		m_bOpened = true;
		return true;
	}

	void _Scepter::close(void)
	{
		m_bOpened = false;

		ScStatus status = scStopStream(m_scDevHandle);
		status = scCloseDevice(&m_scDevHandle);
		m_scDevHandle = 0;
		LOG_I("CloseDevice status: " + i2str(status));

		status = scShutdown();

		DEL(m_pScDevListInfo);
		DEL(m_pScVw);
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

		while (m_pT->bRun())
		{
			if (!m_bOpened)
			{
				if (!open())
				{
					LOG_E("Cannot open Scense");
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
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
			if (m_scfRGB.pFrameData)
			{
				m_mRGB = cv::Mat(m_scfRGB.height, m_scfRGB.width, CV_8UC3, m_scfRGB.pFrameData);
				m_vSizeRGB.x() = m_scfRGB.width;
				m_vSizeRGB.y() = m_scfRGB.height;
			}
		}

		if (m_bDepth && sFr.depth == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_DEPTH_FRAME, &m_scfDepth);
			if (m_scfDepth.pFrameData)
			{
				m_mDepth = cv::Mat(m_scfDepth.height, m_scfDepth.width, CV_16UC1, m_scfDepth.pFrameData);
				m_vSizeD.x() = m_scfDepth.width;
				m_vSizeD.y() = m_scfDepth.height;
			}
		}

		if (m_btRGB && sFr.transformedColor == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_TRANSFORM_COLOR_IMG_TO_DEPTH_SENSOR_FRAME, &m_scfTransformedRGB);
			if (m_scfTransformedRGB.pFrameData)
			{
				m_mtRGB = cv::Mat(m_scfTransformedRGB.height, m_scfTransformedRGB.width, CV_8UC3, m_scfTransformedRGB.pFrameData);
			}
		}

		if (m_btDepth && sFr.transformedDepth == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_TRANSFORM_DEPTH_IMG_TO_COLOR_SENSOR_FRAME, &m_scfTransformedDepth);
			if (m_scfTransformedDepth.pFrameData)
			{
				m_mtDepth = cv::Mat(m_scfTransformedDepth.height, m_scfTransformedDepth.width, CV_16UC1, m_scfTransformedDepth.pFrameData);
			}
		}

		if (m_bIR && sFr.ir == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_IR_FRAME, &m_scfIR);
			if (m_scfIR.pFrameData)
			{
				m_mIR = cv::Mat(m_scfIR.height, m_scfIR.width, CV_8UC1, m_scfIR.pFrameData);
			}
		}

		return true;
	}

	void _Scepter::updateTPP(void)
	{
		while (m_pTpp->bRun())
		{
			m_pTpp->sleepT(0);

			updatePCL();
		}
	}

	void _Scepter::updatePCL(void)
	{
#ifdef WITH_UNIVERSE

		NULL_(m_pPCL);

		const static float s_b = 1.0 / 1000.0;
		const static float c_b = 1.0 / 255.0;

		uint64_t tNow = getApproxTbootUs();

		// Convert Depth frame to World vectors.
		scConvertDepthFrameToPointCloudVector(m_scDevHandle,
											  &m_scfDepth,
											  m_pScVw);

		m_pPCL->frameStart();
		for (int i = 0; i < m_scfDepth.height; i++)
		{
			for (int j = 0; j < m_scfDepth.width; j++)
			{
				int k = i * m_scfDepth.width + j;

				ScVector3f *pV = &m_pScVw[k];
				Vector3f vP(pV->x, pV->y, pV->z);
				vP *= s_b;

				// texture color
				Vector3f vC = Vector3f::Constant(1);
				if (m_scfTransformedRGB.pFrameData)
				{
					uint8_t *pC = &m_scfTransformedRGB.pFrameData[k * sizeof(uint8_t) * 3];
					vC = Vector3f(pC[2], pC[1], pC[0]);
					vC *= c_b;
				}

				m_pPCL->add(vP, vC, tNow);
			}
		}
		m_pPCL->frameStop();
#endif
	}

	bool _Scepter::setToFexposureControlMode(bool bAuto)
	{
		m_scCtrl.m_bAutoExposureToF = bAuto;

		ScStatus ScR = scSetExposureControlMode(m_scDevHandle,
												SC_TOF_SENSOR,
												bAuto ? SC_EXPOSURE_CONTROL_MODE_AUTO
													  : SC_EXPOSURE_CONTROL_MODE_MANUAL);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setToFexposureTime(int tExposure)
	{
		m_scCtrl.m_tExposureToF = tExposure;

		ScStatus ScR = scSetExposureTime(m_scDevHandle,
										 SC_TOF_SENSOR,
										 tExposure);

		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setRGBexposureControlMode(bool bAuto)
	{
		m_scCtrl.m_bAutoExposureRGB = bAuto;

		ScStatus ScR = scSetExposureControlMode(m_scDevHandle,
												SC_COLOR_SENSOR,
												bAuto ? SC_EXPOSURE_CONTROL_MODE_AUTO
													  : SC_EXPOSURE_CONTROL_MODE_MANUAL);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setRGBexposureTime(int tExposure)
	{
		m_scCtrl.m_tExposureRGB = tExposure;

		ScStatus ScR = scSetExposureTime(m_scDevHandle,
										 SC_COLOR_SENSOR,
										 tExposure);

		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setTimeFilter(bool bON, int thr)
	{
		m_scCtrl.m_timeFilter = {thr, bON};

		ScStatus ScR = scSetTimeFilterParams(m_scDevHandle, m_scCtrl.m_timeFilter);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setConfidenceFilter(bool bON, int thr)
	{
		m_scCtrl.m_confidenceFilter = {thr, bON};

		ScStatus ScR = scSetConfidenceFilterParams(m_scDevHandle, m_scCtrl.m_confidenceFilter);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFlyingPixelFilter(bool bON, int thr)
	{
		m_scCtrl.m_flyingPixelFilter = {thr, bON};

		ScStatus ScR = scSetFlyingPixelFilterParams(m_scDevHandle, m_scCtrl.m_flyingPixelFilter);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFillHole(bool bON)
	{
		m_scCtrl.m_bFillHole = bON;

		ScStatus ScR = scSetFillHoleFilterEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setSpatialFilter(bool bON)
	{
		m_scCtrl.m_bSpatialFilter = bON;

		ScStatus ScR = scSetSpatialFilterEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setHDR(bool bON)
	{
		m_scCtrl.m_bHDR = bON;

		ScStatus ScR = scSetHDRModeEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFrameRate(int fps)
	{
		IF_F(fps <= 0);
		m_scCtrl.m_frameRate = fps;

		ScStatus ScR = scSetFrameRate(m_scDevHandle, fps);
		if (ScR == SC_OK)
			m_tFrameInterval = 2 * 1000 / fps;
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setWorkMode(ScWorkMode mode)
	{
		m_scCtrl.m_workMode = mode;

		ScStatus ScR = scSetWorkMode(m_scDevHandle, mode);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setSoftwareTriggerParameter(uint8_t frameCount)
	{
		m_scCtrl.m_softwareTriggerFrameCount = frameCount;

		ScStatus ScR = scSetSoftwareTriggerParameter(m_scDevHandle, frameCount);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setInputSignalParamsForHWTrigger(uint32_t width, uint32_t interval, uint8_t polarity)
	{
		m_scCtrl.m_hwTrigger = {width, interval, polarity};

		ScStatus ScR = scSetInputSignalParamsForHWTrigger(m_scDevHandle, m_scCtrl.m_hwTrigger);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setToFResolution(int width, int height)
	{
		IF_F(width <= 0 || width > UINT16_MAX || height <= 0 || height > UINT16_MAX);
		m_scCtrl.m_resolutionToF = {(uint16_t)width, (uint16_t)height};

		ScStatus ScR = scSetToFResolution(m_scDevHandle, width, height);
		if (ScR == SC_OK)
			m_vSizeD = Vector2i(width, height);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorResolution(int width, int height)
	{
		IF_F(width <= 0 || width > UINT16_MAX || height <= 0 || height > UINT16_MAX);
		m_scCtrl.m_resolutionRGB = {(uint16_t)width, (uint16_t)height};

		ScStatus ScR = scSetColorResolution(m_scDevHandle, width, height);
		if (ScR == SC_OK)
			m_vSizeRGB = Vector2i(width, height);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorPixelFormat(ScPixelFormat pixelFormat)
	{
		m_scCtrl.m_pixelFormat = pixelFormat;

		ScStatus ScR = scSetColorPixelFormat(m_scDevHandle, pixelFormat);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setTransformColorImgToDepthSensorEnabled(bool bON)
	{
		m_scCtrl.m_bTransformRGBToDepth = bON;

		ScStatus ScR = scSetTransformColorImgToDepthSensorEnabled(m_scDevHandle, bON);
		if (ScR == SC_OK)
			m_btRGB = bON;
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setTransformDepthImgToColorSensorEnabled(bool bON)
	{
		m_scCtrl.m_bTransformDepthToRGB = bON;

		ScStatus ScR = scSetTransformDepthImgToColorSensorEnabled(m_scDevHandle, bON);
		if (ScR == SC_OK)
			m_btDepth = bON;
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setIRGMMGain(uint8_t gain)
	{
		m_scCtrl.m_irGMMGain = gain;

		ScStatus ScR = scSetIRGMMGain(m_scDevHandle, gain);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setIRGMMCorrection(bool bON, int thr)
	{
		m_scCtrl.m_irGMMCorrection = {thr, bON};

		ScStatus ScR = scSetIRGMMCorrection(m_scDevHandle, m_scCtrl.m_irGMMCorrection);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorGain(float gain)
	{
		m_scCtrl.m_gainRGB = gain;

		ScStatus ScR = scSetColorGain(m_scDevHandle, gain);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorAECMaxExposureTime(int tExposure)
	{
		m_scCtrl.m_tAECMaxExposureRGB = tExposure;

		ScStatus ScR = scSetColorAECMaxExposureTime(m_scDevHandle, tExposure);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setColorAECROI(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		m_scCtrl.m_aecROIRGBorigin = {x, y};
		m_scCtrl.m_aecROIRGBsize = {width, height};

		ScStatus ScR = scSetColorAECROI(m_scDevHandle, x, y, width, height);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setExposureTimeOfHDR(uint8_t frameIndex, int tExposure)
	{
		m_scCtrl.m_tExposureHDR[frameIndex] = tExposure;

		ScStatus ScR = scSetExposureTimeOfHDR(m_scDevHandle, frameIndex, tExposure);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setWDR(bool bON)
	{
		m_scCtrl.m_bWDR = bON;

		ScStatus ScR = scSetWDRModeEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setExposureTimeOfWDR(uint8_t frameIndex, int tExposure)
	{
		m_scCtrl.m_tExposureWDR[frameIndex] = tExposure;

		ScStatus ScR = scSetExposureTimeOfWDR(m_scDevHandle, frameIndex, tExposure);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setDeviceDHCPEnabled(bool bON)
	{
		m_scCtrl.m_bDHCP = bON;

		ScStatus ScR = scSetDeviceDHCPEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setDeviceIPAddr(const string &ipAddr)
	{
		IF_F(ipAddr.empty() || ipAddr.size() >= 16);
		m_scCtrl.m_deviceIPAddr = ipAddr;

		ScStatus ScR = scSetDeviceIPAddr(m_scDevHandle, ipAddr.c_str(), (int32_t)ipAddr.size());
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setDeviceSubnetMask(const string &mask)
	{
		IF_F(mask.empty() || mask.size() >= 16);
		m_scCtrl.m_deviceSubnetMask = mask;

		ScStatus ScR = scSetDeviceSubnetMask(m_scDevHandle, mask.c_str(), (int32_t)mask.size());
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setRealTimeSyncConfig(const ScTimeSyncConfig &params)
	{
		m_scCtrl.m_timeSync = params;

		ScStatus ScR = scSetRealTimeSyncConfig(m_scDevHandle, params);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setParamsByJson(const string &filePath)
	{
		m_scCtrl.m_paramsJsonFile = filePath;

		ScStatus ScR = scSetParamsByJson(m_scDevHandle, m_scCtrl.m_paramsJsonFile.data());
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setHotPlugStatusCallback(PtrHotPlugStatusCallback pCallback, const void *pUserData)
	{
		m_scCtrl.m_hotPlugCallback = pCallback;
		m_scCtrl.m_pHotPlugUserData = pUserData;

		ScStatus ScR = scSetHotPlugStatusCallback(pCallback, pUserData);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModuleEnabled(bool bON)
	{
		m_scCtrl.m_bAIModule = bON;

		ScStatus ScR = scAIModuleSetEnabled(m_scDevHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModuleWorkMode(ScAIModuleMode mode)
	{
		m_scCtrl.m_aiWorkMode = mode;

		ScStatus ScR = scAIModuleSetWorkMode(m_scDevHandle, mode);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModuleParam(uint32_t paramID, const std::vector<uint8_t> &data)
	{
		IF_F(data.size() > UINT16_MAX);
		m_scCtrl.m_aiParams[paramID] = data;

		std::vector<uint8_t> &param = m_scCtrl.m_aiParams[paramID];
		ScStatus ScR = scAIModuleSetParam(m_scDevHandle, paramID, param.data(), (uint16_t)param.size());
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModuleInputFrameTypeEnabled(ScFrameType frameType, bool bON)
	{
		m_scCtrl.m_aiInputFrames[frameType] = bON;

		ScStatus ScR = scAIModuleSetInputFrameTypeEnabled(m_scDevHandle, frameType, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setAIModulePreviewFrameTypeEnabled(ScFrameType frameType, bool bON)
	{
		m_scCtrl.m_aiPreviewFrames[frameType] = bON;

		ScStatus ScR = scAIModuleSetPreviewFrameTypeEnabled(m_scDevHandle, frameType, bON);
		return (ScR == SC_OK) ? true : false;
	}
}
