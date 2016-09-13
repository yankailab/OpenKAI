/*
 * Camera.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "Camera.h"

namespace kai
{

Camera::Camera()
{
	CamBase();

	m_type = CAM_GENERAL;
}

Camera::~Camera()
{
}

bool Camera::setup(Config* pConfig, string camName)
{
	if (CamBase::setup(pConfig, camName) == false)
		return false;

	string presetDir = "";
	string calibFile;

	CHECK_INFO(pConfig->o("APP")->v("presetDir", &presetDir));
	CHECK_FATAL(pConfig->o(camName)->v("ID", &m_camDeviceID));

	return true;
}

bool Camera::openCamera(void)
{
	int i;

	m_camera.open(m_camDeviceID);
	if (!m_camera.isOpened())
	{
		LOG(ERROR)<< "Cannot open camera:" << m_camDeviceID;
		return false;
	}

	m_camera.set(CV_CAP_PROP_FRAME_WIDTH, m_width);
	m_camera.set(CV_CAP_PROP_FRAME_HEIGHT, m_height);

	//Acquire a frame to determine the actual frame size
	while (!m_camera.read(m_frame));

	m_width = m_frame.cols;
	m_height = m_frame.rows;

	if(m_bCrop)
	{
		i = m_frame.cols - m_cropBB.x;
		if(m_cropBB.width > i)m_cropBB.width = i;

		i = m_frame.rows - m_cropBB.y;
		if(m_cropBB.height > i)m_cropBB.height = i;

		m_width = m_cropBB.width;
		m_height = m_cropBB.height;
	}

	m_centerH = m_width * 0.5;
	m_centerV = m_height * 0.5;

	return true;
}

GpuMat* Camera::readFrame(void)
{
	GpuMat* pSrc;
	GpuMat* pDest;
	GpuMat* pTmp;

	while (!m_camera.read(m_frame));
	m_Gframe.upload(m_frame);

	pSrc = &m_Gframe;
	pDest = &m_Gframe2;

	if (m_bCalibration)
	{
		cuda::remap(*pSrc, *pDest, m_Gmap1, m_Gmap2, INTER_LINEAR);
		SWAP(pSrc, pDest, pTmp);
	}

	if (m_bGimbal)
	{
		cuda::warpAffine(*pSrc, *pDest, m_rotRoll, m_Gframe.size());
		SWAP(pSrc, pDest, pTmp);
	}

	if (m_bCrop)
	{
		*pDest = GpuMat(*pSrc, m_cropBB);
		SWAP(pSrc, pDest, pTmp);
	}

	return pSrc;
}

GpuMat* Camera::getDepthFrame(void)
{
	return NULL;
}

void Camera::release(void)
{
	m_camera.release();
}

} /* namespace kai */
