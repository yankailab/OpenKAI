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

	retCode = pthread_create(&m_pTPP->m_threadID, 0, getTPP, this);
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

		m_fBGR = mCam;
		m_pTPP->wakeUp();

		this->autoFPSto();
	}
}

void _Camera::updateTPP(void)
{
	while (m_bThreadON)
	{
		m_pTPP->sleepTime(0);
		postProcess();
	}
}

}
