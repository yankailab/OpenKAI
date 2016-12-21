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
	_StreamBase();

	m_type = camera;
	m_deviceID = 0;

	m_bCalibration = false;
	m_bFisheye = false;
	m_bCrop = false;
}

_Camera::~_Camera()
{
	complete();
	DEL(m_pBGR);
}

bool _Camera::init(void* pKiss)
{
	CHECK_F(!_StreamBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	string calibFile;

	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("ID", &m_deviceID));

	F_INFO(pK->v("bCrop", &m_bCrop));
	if (m_bCrop != 0)
	{
		F_FATAL_F(pK->v("cropX", &m_cropBB.x));
		F_FATAL_F(pK->v("cropY", &m_cropBB.y));
		F_FATAL_F(pK->v("cropW", &m_cropBB.width));
		F_FATAL_F(pK->v("cropH", &m_cropBB.height));
	}

	F_INFO(pK->v("bCalib", &m_bCalibration));
	F_INFO(pK->v("bFisheye", &m_bFisheye));

	if (pK->v("calibFile", &calibFile))
	{
		FileStorage fs(presetDir + calibFile, FileStorage::READ);
		if (!fs.isOpened())
		{
			LOG_E("Calibration file not found:"<<presetDir<<calibFile);
			m_bCalibration = false;
		}
		else
		{
			fs["camera_matrix"] >> m_cameraMat;
			fs["distortion_coefficients"] >> m_distCoeffs;
			fs.release();

			Mat map1, map2;
			cv::Size imSize(m_width, m_height);

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
				initUndistortRectifyMap(m_cameraMat, m_distCoeffs, Mat(),
						getOptimalNewCameraMatrix(m_cameraMat, m_distCoeffs,
								imSize, 1, imSize, 0), imSize,
						CV_32FC1, map1, map2);
			}

			m_Gmap1.upload(map1);
			m_Gmap2.upload(map2);
		}
	}

	LOG_I("Initialized");
	return true;
}

bool _Camera::link(void)
{
	CHECK_F(!this->_StreamBase::link());
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

	m_camera.set(CV_CAP_PROP_FRAME_WIDTH, m_width);
	m_camera.set(CV_CAP_PROP_FRAME_HEIGHT, m_height);

	//Acquire a frame to determine the actual frame size
	while (!m_camera.read(m_mat));

	m_width = m_mat.cols;
	m_height = m_mat.rows;

	if (m_bCrop)
	{
		int i;
		i = m_mat.cols - m_cropBB.x;
		if (m_cropBB.width > i)
			m_cropBB.width = i;

		i = m_mat.rows - m_cropBB.y;
		if (m_cropBB.height > i)
			m_cropBB.height = i;

		m_width = m_cropBB.width;
		m_height = m_cropBB.height;
	}

	m_centerH = m_width / 2;
	m_centerV = m_height / 2;

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
				LOG_E("Cannot open camera");
				this->sleepThread(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		GpuMat* pSrc;
		GpuMat* pDest;
		GpuMat* pTmp;

		while (!m_camera.read(m_mat));
		m_Gframe.upload(m_mat);

		pSrc = &m_Gframe;
		pDest = &m_Gframe2;

		if (m_bCalibration)
		{
#ifdef USE_OPENCV4TEGRA
			gpu::remap(*pSrc, *pDest, m_Gmap1, m_Gmap2, INTER_LINEAR);
#else
			cuda::remap(*pSrc, *pDest, m_Gmap1, m_Gmap2, INTER_LINEAR);
#endif
			SWAP(pSrc, pDest, pTmp);
		}

		if (m_bGimbal)
		{
#ifdef USE_OPENCV4TEGRA
			gpu::warpAffine(*pSrc, *pDest, m_rotRoll, m_Gframe.size());
#else
			cuda::warpAffine(*pSrc, *pDest, m_rotRoll, m_Gframe.size());
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

void _Camera::complete(void)
{
	this->_StreamBase::complete();
	m_camera.release();
}

bool _Camera::draw(void)
{
	CHECK_F(!this->BASE::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();

	pFrame->update(m_pBGR);
	this->_StreamBase::draw();

	return true;
}

}
