/*
 * _MissionBase.cpp
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#include "_MissionBase.h"

namespace kai
{

_MissionBase::_MissionBase()
{
}

_MissionBase::~_MissionBase()
{
}

bool _MissionBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

//	KISSm(pK,nFIFO);

	return true;
}

bool _MissionBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;

	return true;
}

bool _MissionBase::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	string msg;

	msg = "nFifoW=";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
