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
		m_nDevice = 0;
		m_pScDevListInfo = nullptr;
		m_scDevHandle = 0;

		m_scfRGB = {0};
		m_scfDepth = {0};
		m_scfTransformedRGB = {0};
		m_scfTransformedDepth = {0};
		m_scfIR = {0};

		m_pScVw = NULL;
	}

	_Scepter::~_Scepter()
	{
	}

	bool _Scepter::init(const json& j)
	{
		IF_F(!_RGBDbase::init(j));

		m_scCtrl.m_pixelFormat = j.value("scPixelFormat", m_scCtrl.m_pixelFormat);

		Kiss *pKt = pK->child("threadPP");
		if (pKt->empty())
		{
			LOG_E("threadPP not found");
			return OK_ERR_NOT_FOUND;
		}

		m_pTPP = new _Thread();
		CHECK_d_l_(m_pTPP->init(pKt), DEL(m_pTPP), "threadPP init failed");

		return true;
	}

	bool _Scepter::link(const json& j, ModuleMgr* pM)
	{
		IF_F(!this->_RGBDbase::link(j, pM));
		string n;

		return true;
	}

	bool _Scepter::open(void)
	{
		IF__(m_bOpen, true);

		uint32_t m_nDevice = 0;
		ScStatus status = scGetDeviceCount(&m_nDevice, m_scCtrl.m_tScan);
		if (status != ScStatus::SC_OK)
		{
			LOG_E("ScGetDeviceCount failed");
			return false;
		}

		LOG_I("Get device count: " + i2str(m_nDevice));
		IF_F(m_nDevice == 0);

		// scSetHotPlugStatusCallback(HotPlugStateCallback, nullptr);

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

		scSetFrameRate(m_scDevHandle, (int)this->m_pT->getTargetFPS());
		scSetColorResolution(m_scDevHandle, m_vSizeRGB.x, m_vSizeRGB.y);
		scSetColorPixelFormat(m_scDevHandle, (ScPixelFormat)m_scCtrl.m_pixelFormat);
		scSetTransformColorImgToDepthSensorEnabled(m_scDevHandle, m_btRGB);
		scSetTransformDepthImgToColorSensorEnabled(m_scDevHandle, m_btDepth);
		setToFexposureTime(m_scCtrl.m_bAutoExposureToF, m_scCtrl.m_tExposureToF);
		setToFexposureControlMode(m_scCtrl.m_bAutoExposureToF);
		setRGBexposureTime(m_scCtrl.m_bAutoExposureRGB, m_scCtrl.m_tExposureRGB);
		setRGBexposureControlMode(m_scCtrl.m_bAutoExposureRGB);
		setTimeFilter(m_scCtrl.m_bFilTime, m_scCtrl.m_filTime);
		setConfidenceFilter(m_scCtrl.m_bFilConfidence, m_scCtrl.m_filConfidence);
		setFlyingPixelFilter(m_scCtrl.m_bFilFlyingPix, m_scCtrl.m_filFlyingPix);
		setFillHole(m_scCtrl.m_bFillHole);
		setSpatialFilter(m_scCtrl.m_bSpatialFilter);
		setHDR(m_scCtrl.m_bHDR);
		status = scStartStream(m_scDevHandle);

		m_pScVw = new ScVector3f[m_vSizeRGB.x * m_vSizeRGB.y];

		m_tFrameInterval = 2 * 1000 / this->m_pT->getTargetFPS();
		m_bOpen = true;
		return true;
	}

	void _Scepter::close(void)
	{
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
		return m_pT->start(getUpdate, this);
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

		while (m_pT->bAlive())
		{
			if (!m_bOpen)
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
				// m_fDepth.copy(mDs + m_dOfs);
				// if (m_bDepthShow)
				// {
				//     IF_(m_fDepth.bEmpty());

				//     dispImg = cv::Mat(height, width, CV_16UC1, pData);

				//     dispImg.convertTo(dispImg, CV_8U, 255.0 / slope);
				//     applyColorMap(dispImg, dispImg, cv::COLORMAP_RAINBOW);

				//     Mat mDColor(Size(m_vDsize.x, m_vDsize.y), CV_8UC3, (void *)dColor.get_data(),
				//                 Mat::AUTO_STEP);
				//     m_fDepthShow.copy(mDColor);
				// }
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
		// 	m_bOpen = false;

		IF_F(status != SC_OK);

		if (m_bRGB && sFr.color == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_COLOR_FRAME, &m_scfRGB);
			if (m_scfRGB.pFrameData)
			{
				*m_fRGB.m() = cv::Mat(m_scfRGB.height, m_scfRGB.width, CV_8UC3, m_scfRGB.pFrameData);
				m_vSizeRGB.x = m_scfRGB.width;
				m_vSizeRGB.y = m_scfRGB.height;

				// if (m_psmRGB)
				// 	memcpy(m_psmRGB->p(), m_scfRGB.pFrameData, m_scfRGB.dataLen);
			}
		}

		if (m_bDepth && sFr.depth == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_DEPTH_FRAME, &m_scfDepth);
			if (m_scfDepth.pFrameData)
			{
				*m_fDepth.m() = cv::Mat(m_scfDepth.height, m_scfDepth.width, CV_16UC1, m_scfDepth.pFrameData);
				m_vSizeD.x = m_scfDepth.width;
				m_vSizeD.y = m_scfDepth.height;

				// if (m_psmDepth)
				// 	memcpy(m_psmDepth->p(), m_scfDepth.pFrameData, m_scfDepth.dataLen);
			}
		}

		if (m_btRGB && sFr.transformedColor == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_TRANSFORM_COLOR_IMG_TO_DEPTH_SENSOR_FRAME, &m_scfTransformedRGB);
			if (m_scfTransformedRGB.pFrameData)
			{
				*m_fTfRGB.m() = cv::Mat(m_scfTransformedRGB.height, m_scfTransformedRGB.width, CV_8UC3, m_scfTransformedRGB.pFrameData);

				// if (m_psmTransformedRGB)
				// 	memcpy(m_psmTransformedRGB->p(),
				// 		   m_scfTransformedRGB.pFrameData,
				// 		   m_scfTransformedRGB.dataLen);
			}
		}

		if (m_btDepth && sFr.transformedDepth == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_TRANSFORM_DEPTH_IMG_TO_COLOR_SENSOR_FRAME, &m_scfTransformedDepth);
			if (m_scfTransformedDepth.pFrameData)
			{
				*m_fTfDepth.m() = cv::Mat(m_scfTransformedDepth.height, m_scfTransformedDepth.width, CV_16UC1, m_scfTransformedDepth.pFrameData);

				// if (m_psmTransformedDepth)
				// 	memcpy(m_psmTransformedDepth->p(),
				// 		   m_scfTransformedDepth.pFrameData,
				// 		   m_scfTransformedDepth.dataLen);
			}
		}

		if (m_bIR && sFr.ir == 1)
		{
			status = scGetFrame(m_scDevHandle, SC_IR_FRAME, &m_scfIR);
			if (m_scfIR.pFrameData)
			{
				*m_fIR.m() = cv::Mat(m_scfIR.height, m_scfIR.width, CV_8UC1, m_scfIR.pFrameData);

				// if (m_psmIR)
				// 	memcpy(m_psmIR->p(), m_scfIR.pFrameData, m_scfIR.dataLen);
			}
		}

		return true;
	}

#ifdef WITH_3D
	int _Scepter::getPointCloud(_PCframe* pPCframe, int nPmax)
	{
		NULL__(pPCframe, -1);
		PointCloud* pPC = pPCframe->getNextBuffer();

		const static float s_b = 1.0 / 1000.0;
		const static float c_b = 1.0 / 255.0;

		// Convert Depth frame to World vectors.
		scConvertDepthFrameToPointCloudVector(m_scDevHandle,
											  &m_scfDepth,
											  m_pScVw);

//		Eigen::Affine3d mA = m_A;

		int nPi = 0;
		for (int i = 0; i < m_scfDepth.height; i++)
		{
			for (int j = 0; j < m_scfDepth.width; j++)
			{
				int k = i * m_scfDepth.width + j;

				ScVector3f *pV = &m_pScVw[k];
				Eigen::Vector3d vP(pV->x, pV->y, pV->z);
				vP *= s_b;
				IF_CONT(vP.z() < m_vRangeD.x);
				IF_CONT(vP.z() > m_vRangeD.y);

				// Eigen::Vector3d vPik = Vector3d(
				// 	vP[m_vAxisIdx.x] * m_vAxisK.x,
				// 	vP[m_vAxisIdx.y] * m_vAxisK.y,
				// 	vP[m_vAxisIdx.z] * m_vAxisK.z);
				// vP = mA * vPik;
				pPC->points_.push_back(vP);

				// texture color
				uint8_t *pC = &m_scfTransformedRGB.pFrameData[k * sizeof(uint8_t) * 3];
				Eigen::Vector3d vC(pC[2], pC[1], pC[0]);
				vC *= c_b;
				pPC->colors_.push_back(vC);

				nPi++;
				IF__(nPi >= nPmax, nPi);
			}
		}

		return nPi;
	}
#endif

	bool _Scepter::setToFexposureControlMode(bool bAuto)
	{
		m_scCtrl.m_bAutoExposureToF = bAuto;

		ScStatus ScR = scSetExposureControlMode(m_scDevHandle,
												SC_TOF_SENSOR,
												bAuto ? SC_EXPOSURE_CONTROL_MODE_AUTO
													  : SC_EXPOSURE_CONTROL_MODE_MANUAL);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setToFexposureTime(bool bAuto, int tExposure)
	{
		m_scCtrl.m_bAutoExposureToF = bAuto;
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

	bool _Scepter::setRGBexposureTime(bool bAuto, int tExposure)
	{
		m_scCtrl.m_bAutoExposureRGB = bAuto;
		m_scCtrl.m_tExposureRGB = tExposure;

		ScStatus ScR = scSetExposureTime(m_scDevHandle,
										 SC_COLOR_SENSOR,
										 tExposure);

		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setTimeFilter(bool bON, int thr)
	{
		m_scCtrl.m_bFilTime = bON;
		m_scCtrl.m_filTime = thr;

		ScTimeFilterParams p;
		p.enable = bON;
		p.threshold = thr;
		ScStatus ScR = scSetTimeFilterParams(m_scDevHandle, p);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setConfidenceFilter(bool bON, int thr)
	{
		m_scCtrl.m_bFilConfidence = bON;
		m_scCtrl.m_filConfidence = thr;

		ScConfidenceFilterParams p;
		p.enable = bON;
		p.threshold = thr;
		ScStatus ScR = scSetConfidenceFilterParams(m_scDevHandle, p);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFlyingPixelFilter(bool bON, int thr)
	{
		m_scCtrl.m_bFilFlyingPix = bON;
		m_scCtrl.m_filFlyingPix = thr;

		ScFlyingPixelFilterParams p;
		p.enable = bON;
		p.threshold;
		ScStatus ScR = scSetFlyingPixelFilterParams(m_scDevHandle, p);
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
}
