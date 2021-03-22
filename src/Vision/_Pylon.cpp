/*
 * _Pylon.cpp
 *
 *  Created on: Apr 9, 2018
 *      Author: yankai
 */

#include "_Pylon.h"

#ifdef USE_OPENCV
#ifdef USE_PYLON

namespace kai
{

_Pylon::_Pylon()
{
	m_type = vision_pylon;
	m_SN = "";
	m_pPylonCam = NULL;
	m_pylonFC.OutputPixelFormat = PixelType_BGR8packed;
	m_grabTimeout = 5000;
}

_Pylon::~_Pylon()
{
}

bool _Pylon::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, grabTimeout);
	KISSm(pK, SN);

	return true;
}

bool _Pylon::open(void)
{
	try
	{
		if(m_SN.empty())
		{
			m_pPylonCam = new CInstantCamera(CTlFactory::GetInstance().CreateFirstDevice());
		}
		else
		{
			CDeviceInfo dInfo;
			dInfo.SetSerialNumber(m_SN.c_str());
			m_pPylonCam = new CInstantCamera(CTlFactory::GetInstance().CreateFirstDevice(dInfo));
		}
	}
	catch (const GenericException &e)
	{
		LOG_E(e.GetDescription());
		DEL(m_pPylonCam);
		return false;
	}

	LOG_I("Using model: " + m_pPylonCam->GetDeviceInfo().GetModelName()
			+ ", SN: " + m_pPylonCam->GetDeviceInfo().GetSerialNumber());

	// The parameter MaxNumBuffer can be used to control the count of buffers
	// allocated for grabbing. The default value of this parameter is 10.
	m_pPylonCam->MaxNumBuffer = 10;

	// The camera device is parameterized with a default configuration which
	// sets up free-running continuous acquisition.
	m_pPylonCam->StartGrabbing(GrabStrategy_LatestImageOnly);
	while (!m_pPylonCam->IsGrabbing());

	while (m_pPylonCam->IsGrabbing())
	{
		m_pPylonCam->RetrieveResult(m_grabTimeout, m_pylonGrab, TimeoutHandling_Return);
		if (m_pylonGrab->GrabSucceeded())break;
	}

	m_w = m_pylonGrab->GetWidth();
	m_h = m_pylonGrab->GetHeight();

	m_cW = m_w / 2;
	m_cH = m_h / 2;

	m_bOpen = true;
	return true;
}

bool _Pylon::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _Pylon::update(void)
{
	while(m_pT->bRun())
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				m_pT->sleepTime(SEC_2_USEC);
				continue;
			}
		}

		m_pT->autoFPSfrom();

		while (m_pPylonCam->IsGrabbing())
		{
			m_pPylonCam->RetrieveResult(m_grabTimeout, m_pylonGrab, TimeoutHandling_Return);
			if (m_pylonGrab->GrabSucceeded())break;
		}

		m_pylonFC.Convert(m_pylonImg, m_pylonGrab);
		m_fBGR.copy(Mat(m_h, m_w, CV_8UC3, (uint8_t*) m_pylonImg.GetBuffer()));

		m_pT->autoFPSto();
	}
}

}
#endif
#endif
