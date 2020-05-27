/*
 * _RealSensePC.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_RealSensePC.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#ifdef USE_REALSENSE

namespace kai
{

_RealSensePC::_RealSensePC()
{
	m_type = pointCloud_realsense;
	m_pRS = NULL;
}

_RealSensePC::~_RealSensePC()
{
}

bool _RealSensePC::init(void* pKiss)
{
	IF_F(!_PointCloudBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;

	iName = "";
	pK->v("_RealSense", &iName);
	m_pRS = (_RealSense*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pRS, iName + ": not found");

	return true;
}

bool _RealSensePC::open(void)
{
	NULL_F(m_pRS);

	m_bOpen = m_pRS->isOpened();

	return m_bOpen;
}

void _RealSensePC::close(void)
{
	if (m_threadMode == T_THREAD)
	{
		goSleep();
		while (!bSleeping());
	}

	this->_PointCloudBase::close();
}

bool _RealSensePC::start(void)
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

void _RealSensePC::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				LOG_E("Cannot open RealSensePC");
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

        m_rsPC.map_to(m_pRS->m_rsColor);
        m_rsPoints = m_rsPC.calculate(m_pRS->m_rsDepth);

		this->autoFPSto();
	}
}

void _RealSensePC::draw(void)
{
	this->_PointCloudBase::draw();

}

}
#endif
#endif
#endif
