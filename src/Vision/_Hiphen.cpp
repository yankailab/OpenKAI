/*
 * _Hiphen.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_Hiphen.h"

namespace kai
{

_Hiphen::_Hiphen()
{
	m_type = vision_hiphen;
	m_nSave = 1;
	m_nSnap = 3;

}

_Hiphen::~_Hiphen()
{
	close();
}

bool _Hiphen::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, nSave);
	KISSm(pK, nSnap);

	return true;
}

bool _Hiphen::open(void)
{
	IF_T(m_bOpen);

//	if (!m_camera.isOpened())
//	{
//		LOG_E("Cannot open camera: " + i2str(m_deviceID));
//		return false;
//	}

	Mat mCam;
//	for(int i=0; i<m_nInitRead; i++)
//	{
//		while (!m_camera.read(mCam));
//	}

	m_fBGR.copy(mCam);

	m_w = mCam.cols;
	m_h = mCam.rows;

	m_cW = m_w / 2;
	m_cH = m_h / 2;

	m_bOpen = true;
	return true;
}

void _Hiphen::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Hiphen::start(void)
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

void _Hiphen::update(void)
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
//		while (!m_camera.read(mCam));
		m_fBGR.copy(mCam);

		this->autoFPSto();
	}
}

}
