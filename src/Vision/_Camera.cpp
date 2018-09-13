/*
 * _Camera.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_Camera.h"

namespace kai
{

_Camera::_Camera()
{
	m_type = vision_camera;
	m_deviceID = 0;
}

_Camera::~_Camera()
{
	m_camera.release();
}

bool _Camera::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, deviceID);

	return true;
}

bool _Camera::open(void)
{
	m_camera.open(m_deviceID);
	if (!m_camera.isOpened())
	{
		LOG_E("Cannot open camera: " + i2str(m_deviceID));
		return false;
	}

	m_camera.set(CV_CAP_PROP_FRAME_WIDTH, m_w);
	m_camera.set(CV_CAP_PROP_FRAME_HEIGHT, m_h);
	m_camera.set(CV_CAP_PROP_FPS, m_targetFPS);

	Mat cMat;
	//Acquire a frame to determine the actual frame size
	while (!m_camera.read(cMat));

	m_w = cMat.cols;
	m_h = cMat.rows;

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

		Mat mCam;
		while (!m_camera.read(mCam));
		m_fBGR.copy(mCam);

		this->autoFPSto();
	}
}

}
