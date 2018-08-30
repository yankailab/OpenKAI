/*
 * _LidarSlam.cpp
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#include "_LidarSlam.h"

namespace kai
{

_LidarSlam::_LidarSlam()
{
	m_nDS = 0;
	reset();
}

_LidarSlam::~_LidarSlam()
{
}

bool _LidarSlam::init(void* pKiss)
{
	IF_F(!this->_SlamBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//link
	string iName;

	Kiss** pItr = pK->getChildItr();
	int i=0;
	while (pItr[i])
	{
		Kiss* pP = pItr[i];
		IF_F(m_nDS >= LIDARSLAM_N_LIDAR);
		i++;

		LIDARSLAM_LIDAR* pL = &m_pDS[m_nDS];
		pL->init();

		iName = "";
		F_ERROR_F(pP->v("_DistSensorBase", &iName));
		pL->m_pD = (_DistSensorBase*)(pK->root()->getChildInstByName(&iName));
		if(!pL->m_pD)
		{
			LOG_I("_DistSensorBase not found: " + iName);
			continue;
		}

		m_nDS++;
	}

	m_bReady = true;
	return true;
}

bool _LidarSlam::start(void)
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

void _LidarSlam::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _LidarSlam::detect(void)
{
	IF_(!m_bReady);

	int i;
	for(i=0; i<m_nDS; i++)
	{
		LIDARSLAM_LIDAR* pD = &m_pDS[i];
		pD->update();
	}

	m_pos.x = m_pDS[0].m_d;
	m_pos.y = m_pDS[1].m_d;
	m_pos.z = m_pDS[2].m_d;
}

bool _LidarSlam::draw(void)
{
	IF_F(!this->_SlamBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;

	pWin->tabNext();
	pWin->addMsg(&msg);
	pWin->tabPrev();

	return true;
}

bool _LidarSlam::cli(int& iY)
{
	IF_F(!this->_SlamBase::cli(iY));

	string msg = "x=" + f2str(m_pos.x) +
				 ", y=" + f2str(m_pos.y) +
				 ", z=" + f2str(m_pos.z);

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

void _LidarSlam::reset(void)
{
	this->_SlamBase::reset();
}

}
