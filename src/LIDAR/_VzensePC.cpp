/*
 * _VzensePC.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_VzensePC.h"

namespace kai
{

	_VzensePC::_VzensePC()
	{
		m_nDevice = 0;
		m_deviceURI = "";
		m_pDeviceListInfo = NULL;
		m_deviceHandle = 0;

		m_bOpen = false;
		m_vSize.set(1600, 1200);

		m_bRGB = true;
		m_bDepth = true;
		m_btRGB = false;
		m_btDepth = false;
		m_bIR = false;

		m_vzfRGB = {0};
		m_vzfDepth = {0};
		m_vzfTransformedRGB = {0};
		m_vzfTransformedDepth = {0};
		m_vzfIR = {0};

		//		m_psmTransformedDepth = NULL;

		m_pVzVw = NULL;
		m_vRz.set(0.0, FLT_MAX);
	}

	_VzensePC::~_VzensePC()
	{
	}

	bool _VzensePC::init(void *pKiss)
	{
		IF_F(!_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("URI", &m_deviceURI);
		pK->v("vSize", &m_vSize);

		pK->v("bRGB", &m_bRGB);
		pK->v("bDepth", &m_bDepth);
		pK->v("btRGB", &m_btRGB);
		pK->v("btDepth", &m_btDepth);
		pK->v("bIR", &m_bIR);

		return true;
	}

	bool _VzensePC::open(void)
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
		VZ_SetColorResolution(m_deviceHandle, m_vSize.x, m_vSize.y);
		VZ_SetColorPixelFormat(m_deviceHandle, VzPixelFormatBGR888);
		VZ_SetTransformColorImgToDepthSensorEnabled(m_deviceHandle, m_btRGB);
		VZ_SetTransformDepthImgToColorSensorEnabled(m_deviceHandle, m_btDepth);
		// TODO: set exposure time etc.
		status = VZ_StartStream(m_deviceHandle);

		m_pVzVw = new VzVector3f[m_vSize.x * m_vSize.y];

		m_tWait = 2 * 1000 / this->m_pT->getTargetFPS();
		m_bOpen = true;
		return true;
	}

	void _VzensePC::close(void)
	{
		VzReturnStatus status = VZ_StopStream(m_deviceHandle);
		status = VZ_CloseDevice(&m_deviceHandle);
		m_deviceHandle = 0;
		LOG_I("CloseDevice status: " + i2str(status));

		status = VZ_Shutdown();

		DEL(m_pDeviceListInfo);
		DEL(m_pVzVw);
	}

	bool _VzensePC::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _VzensePC::check(void)
	{
		NULL__(m_pT, -1);
		return this->_PCframe::check();
	}

	void _VzensePC::update(void)
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
				updatePC();
			}

			m_pT->autoFPSto();
		}
	}

	bool _VzensePC::updateVzense(void)
	{
		IF_T(check() < 0);

		VzFrameReady frameReady = {0};
		VzReturnStatus status = VZ_GetFrameReady(m_deviceHandle,
												 m_tWait,
												 &frameReady);
		IF_F(status != VzReturnStatus::VzRetOK);
		IF_F(frameReady.transformedDepth != 1);
		IF_F(frameReady.color != 1);

		status = VZ_GetFrame(m_deviceHandle, VzTransformDepthImgToColorSensorFrame, &m_vzfTransformedDepth);
		IF_F(status != VzReturnStatus::VzRetOK);
		IF_F(!m_vzfTransformedDepth.pFrameData);

		status = VZ_GetFrame(m_deviceHandle, VzColorFrame, &m_vzfRGB);
		IF_F(status != VzReturnStatus::VzRetOK);
		IF_F(!m_vzfRGB.pFrameData);

		// memcpy(m_psmTransformedDepth->p(),
		// 	   m_vzfTransformedDepth.pFrameData,
		// 	   m_vzfTransformedDepth.dataLen);

		PointCloud *pPC = m_sPC.next();
		const static float s_b = 1.0 / 1000.0;
		const static float c_b = 1.0 / 255.0;

		VZ_ConvertDepthFrameToPointCloudVector(m_deviceHandle,
											   &m_vzfTransformedDepth,
											   m_pVzVw);
		// Convert Depth frame to World vectors.

		for (int i = 0; i < m_vzfTransformedDepth.height; i++)
		{
			for (int j = 0; j < m_vzfTransformedDepth.width; j++)
			{
				int k = i * m_vzfTransformedDepth.width + j;

				VzVector3f *pV = &m_pVzVw[k];
				IF_CONT(pV->z < m_vRz.x);
				IF_CONT(pV->z > m_vRz.y);

				Eigen::Vector3d vP(pV->x, pV->y, pV->z);
				vP *= s_b;
				pPC->points_.push_back(vP);

				// texture color
				uint8_t* pC = &m_vzfRGB.pFrameData[k * sizeof(uint8_t) * 3];
				Eigen::Vector3d vC(pC[2], pC[1], pC[0]);
				vC *= c_b;
				pPC->colors_.push_back(vC);
			}
		}

		return true;
	}

}
