/*
 * _Pylon.cpp
 *
 *  Created on: Apr 9, 2018
 *      Author: yankai
 */

#include "_Pylon.h"

#ifdef USE_PYLON

namespace kai
{

_Pylon::_Pylon()
{
	m_type = camera;

	m_bCalibration = false;
	m_bFisheye = false;
	m_bCrop = false;
	m_SN = "";

	m_pPylonCam = NULL;
	m_pylonFC.OutputPixelFormat = PixelType_BGR8packed;
	m_grabTimeout = 5000;
}

_Pylon::~_Pylon()
{
	reset();
}

bool _Pylon::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, bCrop);
	if (m_bCrop != 0)
	{
		F_INFO(pK->v("cropX", &m_cropBB.x));
		F_INFO(pK->v("cropY", &m_cropBB.y));
		F_INFO(pK->v("cropW", &m_cropBB.width));
		F_INFO(pK->v("cropH", &m_cropBB.height));
	}

	KISSm(pK, grabTimeout);
	KISSm(pK, SN);
	KISSm(pK, bCalibration);
	KISSm(pK, bFisheye);
	string calibFile = "";
	F_INFO(pK->v("calibFile", &calibFile));

	if (!calibFile.empty())
	{
		FileStorage fs(calibFile, FileStorage::READ);
		if (!fs.isOpened())
		{
			LOG_E("Calibration file not found:"<<calibFile);
			m_bCalibration = false;
		}
		else
		{
			fs["camera_matrix"] >> m_cameraMat;
			fs["distortion_coefficients"] >> m_distCoeffs;
			fs.release();

			Mat map1, map2;
			cv::Size imSize(m_w, m_h);

			if (m_bFisheye)
			{
				Mat newCamMat;
				fisheye::estimateNewCameraMatrixForUndistortRectify(m_cameraMat,
						m_distCoeffs, imSize, Matx33d::eye(), newCamMat, 1);
				fisheye::initUndistortRectifyMap(m_cameraMat, m_distCoeffs,
						Matx33d::eye(), newCamMat, imSize,
						CV_32F/*CV_16SC2*/, map1, map2);
			}
			else
			{
				m_K = getOptimalNewCameraMatrix(m_cameraMat, m_distCoeffs,
						imSize, 1, imSize, 0);
				initUndistortRectifyMap(m_cameraMat, m_distCoeffs, Mat(), m_K,
						imSize, CV_32FC1, map1, map2);
			}

			m_Gmap1.upload(map1);
			m_Gmap2.upload(map2);

			m_bCalibration = true;
			LOG_I("camera calibration enabled");
		}
	}

	return true;
}

void _Pylon::reset(void)
{
	this->_VisionBase::reset();
}

bool _Pylon::link(void)
{
	IF_F(!this->_VisionBase::link());
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

	LOG_I("Using model: " << m_pPylonCam->GetDeviceInfo().GetModelName() <<
		  ", SN: " << m_pPylonCam->GetDeviceInfo().GetSerialNumber());

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

	if (m_bCrop)
	{
		int i;
		i = m_w - m_cropBB.x;
		if (m_cropBB.width > i)
			m_cropBB.width = i;

		i = m_h - m_cropBB.y;
		if (m_cropBB.height > i)
			m_cropBB.height = i;

		m_w = m_cropBB.width;
		m_h = m_cropBB.height;
	}

	m_cW = m_w / 2;
	m_cH = m_h / 2;

	m_bOpen = true;
	return true;
}

bool _Pylon::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Pylon::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		GpuMat m_Gmat;
		GpuMat m_Gmat2;

		GpuMat* pSrc;
		GpuMat* pDest;
		GpuMat* pTmp;
		Mat cMat;

		while (m_pPylonCam->IsGrabbing())
		{
			m_pPylonCam->RetrieveResult(m_grabTimeout, m_pylonGrab, TimeoutHandling_Return);
			if (m_pylonGrab->GrabSucceeded())break;
		}

		m_pylonFC.Convert(m_pylonImg, m_pylonGrab);
		cMat = Mat(m_h, m_w, CV_8UC3, (uint8_t*) m_pylonImg.GetBuffer());

		m_Gmat.upload(cMat);

		pSrc = &m_Gmat;
		pDest = &m_Gmat2;

		if (m_bCalibration)
		{
			cuda::remap(*pSrc, *pDest, m_Gmap1, m_Gmap2, INTER_LINEAR);
			SWAP(pSrc, pDest, pTmp);
		}

		if (m_bGimbal)
		{
			cuda::warpAffine(*pSrc, *pDest, m_rotRoll, m_Gmat.size());
			SWAP(pSrc, pDest, pTmp);
		}

		if (m_bFlip)
		{
			cuda::flip(*pSrc, *pDest, -1);
			SWAP(pSrc, pDest, pTmp);
		}

		if (m_bCrop)
		{
			*pDest = GpuMat(*pSrc, m_cropBB);
			SWAP(pSrc, pDest, pTmp);
		}

		m_pBGR->update(pSrc);

		if (m_pGray)
			m_pGray->getGrayOf(m_pBGR);

		if (m_pHSV)
			m_pHSV->getHSVOf(m_pBGR);

		this->autoFPSto();
	}
}

}

#endif
