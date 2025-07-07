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
		m_pDeviceListInfo = nullptr;
		m_deviceHandle = 0;

		m_scfRGB = {0};
		m_scfDepth = {0};
		m_scfTransformedRGB = {0};
		m_scfTransformedDepth = {0};
		m_scfIR = {0};

		m_pScVw = NULL;
		m_scSlope = 7495;
	}

	_Scepter::~_Scepter()
	{
	}

	int _Scepter::init(void *pKiss)
	{
		CHECK_(_RGBDbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("scSlope", &m_scSlope);
		pK->v("scPixelFormat", &m_scCtrl.m_pixelFormat);

		Kiss *pKt = pK->child("threadPP");
		if (pKt->empty())
		{
			LOG_E("threadPP not found");
			return OK_ERR_NOT_FOUND;
		}

		m_pTPP = new _Thread();
		CHECK_d_l_(m_pTPP->init(pKt), DEL(m_pTPP), "threadPP init failed");

		return OK_OK;
	}

	int _Scepter::link(void)
	{
		CHECK_(this->_RGBDbase::link());
		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		return OK_OK;
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

		m_pDeviceListInfo = new ScDeviceInfo[m_nDevice];
		status = scGetDeviceInfoList(m_nDevice, m_pDeviceListInfo);
		if (status != ScStatus::SC_OK)
		{
			LOG_E("GetDeviceListInfo failed:" + i2str(status));
			return false;
		}

		m_deviceHandle = 0;
		if (m_devURI.empty())
			m_devURI = string(m_pDeviceListInfo[0].ip);

		LOG_I("Device URI: " + m_devURI);

		status = scOpenDeviceByIP(m_devURI.c_str(), &m_deviceHandle);
		if (status != ScStatus::SC_OK)
		{
			LOG_E("OpenDevice failed");
			return false;
		}

		status = scGetSensorIntrinsicParameters(m_deviceHandle, SC_TOF_SENSOR, &m_cameraParameters);
		cout << "Get scGetSensorIntrinsicParameters status: " << status << endl;
		cout << "ToF Sensor Intinsic: " << endl;
		cout << "Fx: " << m_cameraParameters.fx << endl;
		cout << "Cx: " << m_cameraParameters.cx << endl;
		cout << "Fy: " << m_cameraParameters.fy << endl;
		cout << "Cy: " << m_cameraParameters.cy << endl;
		cout << "ToF Sensor Distortion Coefficient: " << endl;
		cout << "K1: " << m_cameraParameters.k1 << endl;
		cout << "K2: " << m_cameraParameters.k2 << endl;
		cout << "P1: " << m_cameraParameters.p1 << endl;
		cout << "P2: " << m_cameraParameters.p2 << endl;
		cout << "K3: " << m_cameraParameters.k3 << endl;
		cout << "K4: " << m_cameraParameters.k4 << endl;
		cout << "K5: " << m_cameraParameters.k5 << endl;
		cout << "K6: " << m_cameraParameters.k6 << endl;

		const int nBfw = 64;
		char fw[nBfw] = {0};
		scGetFirmwareVersion(m_deviceHandle, fw, nBfw);
		LOG_I("fw  ==  " + string(fw));
		LOG_I("sn  ==  " + string(m_pDeviceListInfo[0].serialNumber));

		scSetFrameRate(m_deviceHandle, (int)this->m_pT->getTargetFPS());
		scSetColorResolution(m_deviceHandle, m_vSizeRGB.x, m_vSizeRGB.y);
		scSetColorPixelFormat(m_deviceHandle, (ScPixelFormat)m_scCtrl.m_pixelFormat);
		scSetTransformColorImgToDepthSensorEnabled(m_deviceHandle, m_btRGB);
		scSetTransformDepthImgToColorSensorEnabled(m_deviceHandle, m_btDepth);
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
		status = scStartStream(m_deviceHandle);

		m_pScVw = new ScVector3f[m_vSizeRGB.x * m_vSizeRGB.y];

		m_tFrameInterval = 2 * 1000 / this->m_pT->getTargetFPS();
		m_bOpen = true;
		return true;
	}

	void _Scepter::close(void)
	{
		ScStatus status = scStopStream(m_deviceHandle);
		status = scCloseDevice(&m_deviceHandle);
		m_deviceHandle = 0;
		LOG_I("CloseDevice status: " + i2str(status));

		status = scShutdown();

		DEL(m_pDeviceListInfo);
		DEL(m_pScVw);
	}

	int _Scepter::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTPP, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdate, this));
		return m_pTPP->start(getTPP, this);
	}

	int _Scepter::check(void)
	{
		NULL__(m_pTPP, OK_ERR_NULLPTR);

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
				m_pTPP->run();
				// updatePC();
			}
			else
			{
				m_bOpen = false;
			}
		}
	}

	bool _Scepter::updateScRGBD(void)
	{
		ScFrameReady sFr = {0};
		ScStatus status = scGetFrameReady(m_deviceHandle,
										  m_tFrameInterval,
										  &sFr);
//		IF_F(status != SC_OK);

		if (m_bRGB && sFr.color == 1)
		{
			status = scGetFrame(m_deviceHandle, SC_COLOR_FRAME, &m_scfRGB);
			if (m_scfRGB.pFrameData)
			{
				if (m_psmRGB)
					memcpy(m_psmRGB->p(), m_scfRGB.pFrameData, m_scfRGB.dataLen);

				*m_fRGB.m() = cv::Mat(m_scfRGB.height, m_scfRGB.width, CV_8UC3, m_scfRGB.pFrameData);
				m_vSizeRGB.x = m_scfRGB.width;
				m_vSizeRGB.y = m_scfRGB.height;
			}
		}

		if (m_bDepth && sFr.depth == 1)
		{
			status = scGetFrame(m_deviceHandle, SC_DEPTH_FRAME, &m_scfDepth);
			if (m_scfDepth.pFrameData)
			{
				if (m_psmDepth)
					memcpy(m_psmDepth->p(), m_scfDepth.pFrameData, m_scfDepth.dataLen);

				*m_fDepth.m() = cv::Mat(m_scfDepth.height, m_scfDepth.width, CV_16UC1, m_scfDepth.pFrameData);
			}
		}

		if (m_btRGB && sFr.transformedColor == 1)
		{
			status = scGetFrame(m_deviceHandle, SC_TRANSFORM_COLOR_IMG_TO_DEPTH_SENSOR_FRAME, &m_scfTransformedRGB);
			if (m_scfTransformedRGB.pFrameData)
			{
				if (m_psmTransformedRGB)
					memcpy(m_psmTransformedRGB->p(),
						   m_scfTransformedRGB.pFrameData,
						   m_scfTransformedRGB.dataLen);

//				imageMat = cv::Mat(m_scfTransformedRGB.height, m_scfTransformedRGB.width, CV_8UC3, m_scfTransformedRGB.pFrameData);
			}
		}

		if (m_btDepth && sFr.transformedDepth == 1)
		{
			status = scGetFrame(m_deviceHandle, SC_TRANSFORM_DEPTH_IMG_TO_COLOR_SENSOR_FRAME, &m_scfTransformedDepth);
			if (m_scfTransformedDepth.pFrameData)
			{
				if (m_psmTransformedDepth)
					memcpy(m_psmTransformedDepth->p(),
						   m_scfTransformedDepth.pFrameData,
						   m_scfTransformedDepth.dataLen);

//				imageMat = cv::Mat(m_scfTransformedDepth.height, m_scfTransformedDepth.width, CV_16UC1, m_scfTransformedDepth.pFrameData);
			}
		}

		if (m_bIR && sFr.ir == 1)
		{
			status = scGetFrame(m_deviceHandle, SC_IR_FRAME, &m_scfIR);
			if (m_scfIR.pFrameData)
			{
				if (m_psmIR)
					memcpy(m_psmIR->p(), m_scfIR.pFrameData, m_scfIR.dataLen);

//				imageMat = cv::Mat(irFrame.height, irFrame.width, CV_8UC1, irFrame.pFrameData);
			}
		}

		return true;
	}

	bool _Scepter::updatePointCloud(void)
	{
		ScStatus status;

		// Eigen::Affine3d mA = m_A;
		// IF_F(status != ScStatus::SC_OK);
		// IF_F(sFr.transformedColor != 1);
		// IF_F(sFr.depth != 1);

		// status = scGetFrame(m_deviceHandle, ScDepthFrame, &m_scfDepth);
		// IF_F(status != ScStatus::SC_OK);
		// IF_F(!m_scfDepth.pFrameData);

		// status = scGetFrame(m_deviceHandle, ScTransformColorImgToDepthSensorFrame, &m_scfRGB);
		// IF_F(status != ScStatus::SC_OK);
		// IF_F(!m_scfRGB.pFrameData);

		// PointCloud *pPC = m_sPC.next();
		// const static float s_b = 1.0 / 1000.0;
		// const static float c_b = 1.0 / 255.0;

		// // Convert Depth frame to World vectors.
		// scConvertDepthFrameToPointCloudVector(m_deviceHandle,
		// 									   &m_scfDepth,
		// 									   m_pScVw);

		// int nPi = 0;
		// for (int i = 0; i < m_scfDepth.height; i++)
		// {
		// 	for (int j = 0; j < m_scfDepth.width; j++)
		// 	{
		// 		int k = i * m_scfDepth.width + j;

		// 		ScVector3f *pV = &m_pScVw[k];
		// 		Eigen::Vector3d vP(pV->x, pV->y, pV->z);
		// 		vP *= s_b;
		// 		IF_CONT(vP.z() < m_scCtrl.m_vRz.x);
		// 		IF_CONT(vP.z() > m_scCtrl.m_vRz.y);

		// 		Eigen::Vector3d vPik = Vector3d(
		// 			vP[m_vAxisIdx.x] * m_vAxisK.x,
		// 			vP[m_vAxisIdx.y] * m_vAxisK.y,
		// 			vP[m_vAxisIdx.z] * m_vAxisK.z);
		// 		vP = mA * vPik;
		// 		pPC->points_.push_back(vP);

		// 		// texture color
		// 		uint8_t *pC = &m_scfRGB.pFrameData[k * sizeof(uint8_t) * 3];
		// 		Eigen::Vector3d vC(pC[2], pC[1], pC[0]);
		// 		vC *= c_b;
		// 		pPC->colors_.push_back(vC);

		// 		nPi++;
		// 		if (nPi >= m_nPresv)
		// 			break;
		// 	}

		// 	if (nPi >= m_nPresv)
		// 		break;
		// }

		return true;
	}

	void _Scepter::updateTPP(void)
	{
		while (m_pTPP->bAlive())
		{
			m_pTPP->sleepT(0);

			//            m_fDepth.copy(mDs + m_dOfs);
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

	bool _Scepter::setToFexposureControlMode(bool bAuto)
	{
		m_scCtrl.m_bAutoExposureToF = bAuto;

		ScStatus ScR = scSetExposureControlMode(m_deviceHandle,
												SC_TOF_SENSOR,
												bAuto ? SC_EXPOSURE_CONTROL_MODE_AUTO
													  : SC_EXPOSURE_CONTROL_MODE_MANUAL);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setToFexposureTime(bool bAuto, int tExposure)
	{
		m_scCtrl.m_bAutoExposureToF = bAuto;
		m_scCtrl.m_tExposureToF = tExposure;

		ScStatus ScR = scSetExposureTime(m_deviceHandle,
										 SC_TOF_SENSOR,
										 tExposure);

		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setRGBexposureControlMode(bool bAuto)
	{
		m_scCtrl.m_bAutoExposureRGB = bAuto;

		ScStatus ScR = scSetExposureControlMode(m_deviceHandle,
												SC_COLOR_SENSOR,
												bAuto ? SC_EXPOSURE_CONTROL_MODE_AUTO
													  : SC_EXPOSURE_CONTROL_MODE_MANUAL);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setRGBexposureTime(bool bAuto, int tExposure)
	{
		m_scCtrl.m_bAutoExposureRGB = bAuto;
		m_scCtrl.m_tExposureRGB = tExposure;

		ScStatus ScR = scSetExposureTime(m_deviceHandle,
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
		ScStatus ScR = scSetTimeFilterParams(m_deviceHandle, p);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setConfidenceFilter(bool bON, int thr)
	{
		m_scCtrl.m_bFilConfidence = bON;
		m_scCtrl.m_filConfidence = thr;

		ScConfidenceFilterParams p;
		p.enable = bON;
		p.threshold = thr;
		ScStatus ScR = scSetConfidenceFilterParams(m_deviceHandle, p);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFlyingPixelFilter(bool bON, int thr)
	{
		m_scCtrl.m_bFilFlyingPix = bON;
		m_scCtrl.m_filFlyingPix = thr;

		ScFlyingPixelFilterParams p;
		p.enable = bON;
		p.threshold;
		ScStatus ScR = scSetFlyingPixelFilterParams(m_deviceHandle, p);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setFillHole(bool bON)
	{
		m_scCtrl.m_bFillHole = bON;

		ScStatus ScR = scSetFillHoleFilterEnabled(m_deviceHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setSpatialFilter(bool bON)
	{
		m_scCtrl.m_bSpatialFilter = bON;

		ScStatus ScR = scSetSpatialFilterEnabled(m_deviceHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}

	bool _Scepter::setHDR(bool bON)
	{
		m_scCtrl.m_bHDR = bON;

		ScStatus ScR = scSetHDRModeEnabled(m_deviceHandle, bON);
		return (ScR == SC_OK) ? true : false;
	}
}
