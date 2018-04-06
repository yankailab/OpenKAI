/*
 * Camera.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_Camera.h"

namespace kai
{

_Camera::_Camera()
{
	m_type = camera;
	m_deviceID = 0;

	m_bCalibration = false;
	m_bFisheye = false;
	m_bCrop = false;
}

_Camera::~_Camera()
{
	reset();
}

bool _Camera::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, deviceID);
	KISSm(pK, bCrop);
	if (m_bCrop != 0)
	{
		F_INFO(pK->v("cropX", &m_cropBB.x));
		F_INFO(pK->v("cropY", &m_cropBB.y));
		F_INFO(pK->v("cropW", &m_cropBB.width));
		F_INFO(pK->v("cropH", &m_cropBB.height));
	}

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
				m_K = getOptimalNewCameraMatrix(m_cameraMat, m_distCoeffs, imSize, 1, imSize, 0);
				initUndistortRectifyMap(m_cameraMat, m_distCoeffs, Mat(), m_K, imSize, CV_32FC1, map1, map2);
			}

			m_Gmap1.upload(map1);
			m_Gmap2.upload(map2);

			m_bCalibration = true;
			LOG_I("camera calibration enabled");
		}
	}

	LOG_I("Initialized");
	return true;
}

void _Camera::reset(void)
{
	this->_VisionBase::reset();
	m_camera.release();
}

bool _Camera::link(void)
{
	IF_F(!this->_VisionBase::link());
	return true;
}

bool _Camera::open(void)
{
	m_camera.open(m_deviceID);
	if (!m_camera.isOpened())
	{
		LOG_E("Cannot open camera:" << m_deviceID);
		return false;
	}

	m_camera.set(CV_CAP_PROP_FRAME_WIDTH, m_w);
	m_camera.set(CV_CAP_PROP_FRAME_HEIGHT, m_h);

	Mat cMat;
	//Acquire a frame to determine the actual frame size
	while (!m_camera.read(cMat));

	m_w = cMat.cols;
	m_h = cMat.rows;

	if (m_bCrop)
	{
		int i;
		i = cMat.cols - m_cropBB.x;
		if (m_cropBB.width > i)
			m_cropBB.width = i;

		i = cMat.rows - m_cropBB.y;
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

bool _Camera::start(void)
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

void _Camera::update(void)
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

		while (!m_camera.read(cMat));
		m_Gmat.upload(cMat);

		pSrc = &m_Gmat;
		pDest = &m_Gmat2;

		if (m_bCalibration)
		{
#ifdef USE_OPENCV2X
			gpu::remap(*pSrc, *pDest, m_Gmap1, m_Gmap2, INTER_LINEAR);
#else
			cuda::remap(*pSrc, *pDest, m_Gmap1, m_Gmap2, INTER_LINEAR);
#endif
			SWAP(pSrc, pDest, pTmp);
		}

		if (m_bGimbal)
		{
#ifdef USE_OPENCV2X
			gpu::warpAffine(*pSrc, *pDest, m_rotRoll, m_Gmat.size());
#else
			cuda::warpAffine(*pSrc, *pDest, m_rotRoll, m_Gmat.size());
#endif
			SWAP(pSrc, pDest, pTmp);
		}

		if (m_bFlip)
		{
#ifdef USE_OPENCV2X
			gpu::flip(*pSrc, *pDest, -1);
#else
			cuda::flip(*pSrc, *pDest, -1);
#endif
			SWAP(pSrc, pDest, pTmp);
		}

		if (m_bCrop)
		{
			*pDest = GpuMat(*pSrc, m_cropBB);
			SWAP(pSrc, pDest, pTmp);
		}

		m_pBGR->update(pSrc);

		if(m_pGray)
			m_pGray->getGrayOf(m_pBGR);

		if(m_pHSV)
			m_pHSV->getHSVOf(m_pBGR);

		this->autoFPSto();
	}
}

}
