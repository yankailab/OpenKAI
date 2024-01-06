/*
 * _Vzense.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_Vzense.h"

namespace kai
{

	_Vzense::_Vzense()
	{
		m_nDevice = 0;
		m_pDeviceListInfo = NULL;
		m_deviceHandle = 0;

		m_vzfRGB = {0};
		m_vzfDepth = {0};
		m_vzfTransformedRGB = {0};
		m_vzfTransformedDepth = {0};
		m_vzfIR = {0};

		m_pVzVw = NULL;
	}

	_Vzense::~_Vzense()
	{
	}

	bool _Vzense::init(void *pKiss)
	{
		IF_F(!_RGBDbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pKt = pK->child("threadPP");
		IF_F(pKt->empty());

		m_pTPP = new _Thread();
		if (!m_pTPP->init(pKt))
		{
			DEL(m_pTPP);
			return false;
		}

		return true;
	}

	bool _Vzense::link(void)
	{
		IF_F(!this->_RGBDbase::link());
		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		return true;
	}

	bool _Vzense::open(void)
	{
		IF_T(m_bOpen);

		uint32_t m_nDevice = 0;
		VzReturnStatus status = VZ_GetDeviceCount(&m_nDevice);
		if (status != VzReturnStatus::VzRetOK)
		{
			LOG_E("VzGetDeviceCount failed");
			return false;
		}

		LOG_I("Get device count: " + i2str(m_nDevice));
		IF_F(m_nDevice == 0);

		//        VZ_SetHotPlugStatusCallback(HotPlugStateCallback, nullptr);

		m_pDeviceListInfo = new VzDeviceInfo[m_nDevice];
		status = VZ_GetDeviceInfoList(m_nDevice, m_pDeviceListInfo);
		if (status != VzReturnStatus::VzRetOK)
		{
			LOG_E("GetDeviceListInfo failed:" + i2str(status));
			return false;
		}

		m_deviceHandle = 0;
		if (m_deviceURI.empty())
			m_deviceURI = string(m_pDeviceListInfo[0].uri);

		status = VZ_OpenDeviceByUri(m_deviceURI.c_str(), &m_deviceHandle);
		if (status != VzReturnStatus::VzRetOK)
		{
			LOG_E("OpenDevice failed");
			return false;
		}

		status = VZ_GetSensorIntrinsicParameters(m_deviceHandle, VzToFSensor, &m_cameraParameters);
		cout << "Get VZ_GetSensorIntrinsicParameters status: " << status << endl;
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
		VZ_GetFirmwareVersion(m_deviceHandle, fw, nBfw);
		LOG_I("fw  ==  " + string(fw));
		LOG_I("sn  ==  " + string(m_pDeviceListInfo[0].serialNumber));

		VZ_SetFrameRate(m_deviceHandle, (int)this->m_pT->getTargetFPS());
		VZ_SetColorResolution(m_deviceHandle, m_vSizeRGB.x, m_vSizeRGB.y);
		VZ_SetColorPixelFormat(m_deviceHandle, VzPixelFormatBGR888);
		VZ_SetTransformColorImgToDepthSensorEnabled(m_deviceHandle, m_btRGB);
		VZ_SetTransformDepthImgToColorSensorEnabled(m_deviceHandle, m_btDepth);
		setToFexposureTime(m_vzCtrl.m_bAutoExposureToF, m_vzCtrl.m_tExposureToF);
		setToFexposureControlMode(m_vzCtrl.m_bAutoExposureToF);
		setRGBexposureTime(m_vzCtrl.m_bAutoExposureRGB, m_vzCtrl.m_tExposureToF);
		setRGBexposureControlMode(m_vzCtrl.m_bAutoExposureRGB);
		setTimeFilter(m_vzCtrl.m_bFilTime, m_vzCtrl.m_filTime);
		setConfidenceFilter(m_vzCtrl.m_bFilConfidence, m_vzCtrl.m_filConfidence);
		setFlyingPixelFilter(m_vzCtrl.m_bFilFlyingPix, m_vzCtrl.m_filFlyingPix);
		setFillHole(m_vzCtrl.m_bFillHole);
		setSpatialFilter(m_vzCtrl.m_bSpatialFilter);
		setHDR(m_vzCtrl.m_bHDR);
		status = VZ_StartStream(m_deviceHandle);

		m_pVzVw = new VzVector3f[m_vSizeRGB.x * m_vSizeRGB.y];

		m_tFrameInterval = 2 * 1000 / this->m_pT->getTargetFPS();
		m_bOpen = true;
		return true;
	}

	void _Vzense::close(void)
	{
		VzReturnStatus status = VZ_StopStream(m_deviceHandle);
		status = VZ_CloseDevice(&m_deviceHandle);
		m_deviceHandle = 0;
		LOG_I("CloseDevice status: " + i2str(status));

		status = VZ_Shutdown();

		DEL(m_pDeviceListInfo);
		DEL(m_pVzVw);
	}

	bool _Vzense::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTPP);
		IF_F(!m_pT->start(getUpdate, this));
		return m_pTPP->start(getTPP, this);
	}

	int _Vzense::check(void)
	{
		NULL__(m_pTPP, -1);

		return this->_RGBDbase::check();
	}

	void _Vzense::update(void)
	{
		VzReturnStatus status = VZ_Initialize();
		if (status != VzReturnStatus::VzRetOK)
		{
			LOG_E("VzInitialize failed");
			return;
		}

		while (m_pT->bRun())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					LOG_E("Cannot open Vzense");
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			if (updateVzense())
			{
				m_pTPP->wakeUp();
				//				updatePC();
			}
			else
			{
				m_pT->sleepT(SEC_2_USEC);
				m_bOpen = false;
			}

			m_pT->autoFPSto();
		}
	}

	bool _Vzense::updateVzense(void)
	{
		IF_T(check() < 0);

		VzFrameReady frameReady = {0};
		VzReturnStatus status = VZ_GetFrameReady(m_deviceHandle,
												 m_tFrameInterval,
												 &frameReady);

		updateRGBD(frameReady);
		updatePointCloud(frameReady);

		return true;
	}

	bool _Vzense::updateRGBD(const VzFrameReady &vfr)
	{
		VzReturnStatus status;

		if (m_bRGB && vfr.color == 1)
		{
			status = VZ_GetFrame(m_deviceHandle, VzColorFrame, &m_vzfRGB);
			if (m_vzfRGB.pFrameData)
				memcpy(m_psmRGB->p(), m_vzfRGB.pFrameData, m_vzfRGB.dataLen);
		}

		if (m_btRGB && vfr.transformedColor == 1)
		{
			status = VZ_GetFrame(m_deviceHandle, VzTransformColorImgToDepthSensorFrame, &m_vzfTransformedRGB);
			if (m_vzfTransformedRGB.pFrameData)
				memcpy(m_psmTransformedRGB->p(),
					   m_vzfTransformedRGB.pFrameData,
					   m_vzfTransformedRGB.dataLen);
		}

		if (m_bDepth && vfr.depth == 1)
		{
			status = VZ_GetFrame(m_deviceHandle, VzDepthFrame, &m_vzfDepth);
			if (m_vzfDepth.pFrameData)
				memcpy(m_psmDepth->p(), m_vzfDepth.pFrameData, m_vzfDepth.dataLen);
		}

		if (m_btDepth && vfr.transformedDepth == 1)
		{
			status = VZ_GetFrame(m_deviceHandle, VzTransformDepthImgToColorSensorFrame, &m_vzfTransformedDepth);
			if (m_vzfTransformedDepth.pFrameData)
				memcpy(m_psmTransformedDepth->p(),
					   m_vzfTransformedDepth.pFrameData,
					   m_vzfTransformedDepth.dataLen);
		}

		if (m_bIR && vfr.ir == 1)
		{
			status = VZ_GetFrame(m_deviceHandle, VzIRFrame, &m_vzfIR);
			if (m_vzfIR.pFrameData)
				memcpy(m_psmIR->p(), m_vzfIR.pFrameData, m_vzfIR.dataLen);
		}

		return true;
	}

	bool _Vzense::updatePointCloud(const VzFrameReady &vfr)
	{
		VzReturnStatus status;

		// Eigen::Affine3d mA = m_A;
		// IF_F(status != VzReturnStatus::VzRetOK);
		// IF_F(vfr.transformedColor != 1);
		// IF_F(vfr.depth != 1);

		// status = VZ_GetFrame(m_deviceHandle, VzDepthFrame, &m_vzfDepth);
		// IF_F(status != VzReturnStatus::VzRetOK);
		// IF_F(!m_vzfDepth.pFrameData);

		// status = VZ_GetFrame(m_deviceHandle, VzTransformColorImgToDepthSensorFrame, &m_vzfRGB);
		// IF_F(status != VzReturnStatus::VzRetOK);
		// IF_F(!m_vzfRGB.pFrameData);

		// PointCloud *pPC = m_sPC.next();
		// const static float s_b = 1.0 / 1000.0;
		// const static float c_b = 1.0 / 255.0;

		// // Convert Depth frame to World vectors.
		// VZ_ConvertDepthFrameToPointCloudVector(m_deviceHandle,
		// 									   &m_vzfDepth,
		// 									   m_pVzVw);

		// int nPi = 0;
		// for (int i = 0; i < m_vzfDepth.height; i++)
		// {
		// 	for (int j = 0; j < m_vzfDepth.width; j++)
		// 	{
		// 		int k = i * m_vzfDepth.width + j;

		// 		VzVector3f *pV = &m_pVzVw[k];
		// 		Eigen::Vector3d vP(pV->x, pV->y, pV->z);
		// 		vP *= s_b;
		// 		IF_CONT(vP.z() < m_vzCtrl.m_vRz.x);
		// 		IF_CONT(vP.z() > m_vzCtrl.m_vRz.y);

		// 		Eigen::Vector3d vPik = Vector3d(
		// 			vP[m_vAxisIdx.x] * m_vAxisK.x,
		// 			vP[m_vAxisIdx.y] * m_vAxisK.y,
		// 			vP[m_vAxisIdx.z] * m_vAxisK.z);
		// 		vP = mA * vPik;
		// 		pPC->points_.push_back(vP);

		// 		// texture color
		// 		uint8_t *pC = &m_vzfRGB.pFrameData[k * sizeof(uint8_t) * 3];
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

	void _Vzense::updateTPP(void)
	{
		while (m_pTPP->bRun())
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

	bool _Vzense::setToFexposureControlMode(bool bAuto)
	{
		m_vzCtrl.m_bAutoExposureToF = bAuto;

		VzReturnStatus vzR = VZ_SetExposureControlMode(m_deviceHandle,
													   VzToFSensor,
													   bAuto ? VzExposureControlMode_Auto
															 : VzExposureControlMode_Manual);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setToFexposureTime(bool bAuto, int tExposure)
	{
		m_vzCtrl.m_bAutoExposureToF = bAuto;
		m_vzCtrl.m_tExposureToF = tExposure;

		VzExposureTimeParams p;
		p.mode = bAuto ? VzExposureControlMode_Auto
					   : VzExposureControlMode_Manual;
		p.exposureTime = tExposure;
		VzReturnStatus vzR = VZ_SetExposureTime(m_deviceHandle,
												VzToFSensor,
												p);

		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setRGBexposureControlMode(bool bAuto)
	{
		m_vzCtrl.m_bAutoExposureRGB = bAuto;

		VzReturnStatus vzR = VZ_SetExposureControlMode(m_deviceHandle,
													   VzColorSensor,
													   bAuto ? VzExposureControlMode_Auto
															 : VzExposureControlMode_Manual);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setRGBexposureTime(bool bAuto, int tExposure)
	{
		m_vzCtrl.m_bAutoExposureRGB = bAuto;
		m_vzCtrl.m_tExposureRGB = tExposure;

		VzExposureTimeParams p;
		p.mode = bAuto ? VzExposureControlMode_Auto
					   : VzExposureControlMode_Manual;
		p.exposureTime = tExposure;
		VzReturnStatus vzR = VZ_SetExposureTime(m_deviceHandle,
												VzColorSensor,
												p);

		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setTimeFilter(bool bON, int thr)
	{
		m_vzCtrl.m_bFilTime = bON;
		m_vzCtrl.m_filTime = thr;

		VzTimeFilterParams p;
		p.enable = bON;
		p.threshold = thr;
		VzReturnStatus vzR = VZ_SetTimeFilterParams(m_deviceHandle, p);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setConfidenceFilter(bool bON, int thr)
	{
		m_vzCtrl.m_bFilConfidence = bON;
		m_vzCtrl.m_filConfidence = thr;

		VzConfidenceFilterParams p;
		p.enable = bON;
		p.threshold = thr;
		VzReturnStatus vzR = VZ_SetConfidenceFilterParams(m_deviceHandle, p);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setFlyingPixelFilter(bool bON, int thr)
	{
		m_vzCtrl.m_bFilFlyingPix = bON;
		m_vzCtrl.m_filFlyingPix = thr;

		VzFlyingPixelFilterParams p;
		p.enable = bON;
		p.threshold;
		VzReturnStatus vzR = VZ_SetFlyingPixelFilterParams(m_deviceHandle, p);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setFillHole(bool bON)
	{
		m_vzCtrl.m_bFillHole = bON;

		VzReturnStatus vzR = VZ_SetFillHoleFilterEnabled(m_deviceHandle, bON);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setSpatialFilter(bool bON)
	{
		m_vzCtrl.m_bSpatialFilter = bON;

		VzReturnStatus vzR = VZ_SetSpatialFilterEnabled(m_deviceHandle, bON);
		return (vzR == VzRetOK) ? true : false;
	}

	bool _Vzense::setHDR(bool bON)
	{
		m_vzCtrl.m_bHDR = bON;

		VzReturnStatus vzR = VZ_SetHDRModeEnabled(m_deviceHandle, bON);
		return (vzR == VzRetOK) ? true : false;
	}

}
