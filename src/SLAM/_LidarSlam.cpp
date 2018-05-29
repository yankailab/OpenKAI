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
	m_pDS = NULL;
}

_LidarSlam::~_LidarSlam()
{
}

bool _LidarSlam::init(void* pKiss)
{
	IF_F(!this->_SlamBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

//	KISSm(pK, width);

	return true;
}

bool _LidarSlam::link(void)
{
	IF_F(!this->_SlamBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

//	iName = "";
//	pK->v("_VisionBase", &iName);
//	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

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

	string msg;

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
