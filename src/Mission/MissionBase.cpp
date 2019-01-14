/*
 * MissionBase.cpp
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#include "MissionBase.h"

namespace kai
{

MissionBase::MissionBase()
{
	m_nextMission = "";
	m_tTimeout = 0;

	reset();
}

MissionBase::~MissionBase()
{
}

bool MissionBase::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,nextMission);
	KISSm(pK,tTimeout);

	return true;
}

bool MissionBase::missionStart(void)
{
	m_tStart = getTimeUsec();
	return true;
}

bool MissionBase::update(void)
{
	return false;
}

void MissionBase::reset(void)
{
	m_tStart = 0;
}

bool MissionBase::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;

	return true;
}

bool MissionBase::console(int& iY)
{
	IF_F(!this->BASE::console(iY));

	return true;
}

}
