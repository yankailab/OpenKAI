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
	m_nInitRead = 1;
	m_bResetCam = false;
}

_Camera::~_Camera()
{
	close();
}

bool _Camera::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("deviceID",&m_deviceID);
	pK->v("nInitRead",&m_nInitRead);
	pK->v("bResetCam",&m_bResetCam);

	return true;
}

bool _Camera::open(void)
{
	IF_T(m_bOpen);

	m_camera.open(m_deviceID);
	if (!m_camera.isOpened())
	{
		LOG_E("Cannot open camera: " + i2str(m_deviceID));
		return false;
	}

	m_camera.set(CAP_PROP_FRAME_WIDTH, m_w);
	m_camera.set(CAP_PROP_FRAME_HEIGHT, m_h);
	m_camera.set(CAP_PROP_FPS, m_targetFPS);

	Mat mCam;
	for(int i=0; i<m_nInitRead; i++)
	{
		while (!m_camera.read(mCam));
	}
	m_fBGR.copy(mCam);

	m_w = mCam.cols;
	m_h = mCam.rows;

	m_cW = m_w / 2;
	m_cH = m_h / 2;

	m_bOpen = true;
	return true;
}

void _Camera::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	m_camera.release();
	this->_VisionBase::close();
}

bool _Camera::start(void)
{
	IF_F(!this->_ThreadBase::start());

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

		if(m_bResetCam)
		{
			m_camera.release();
			m_bOpen = false;
		}

		this->autoFPSto();
	}
}

}
