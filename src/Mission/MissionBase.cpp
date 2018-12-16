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
}

MissionBase::~MissionBase()
{
}

bool MissionBase::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,nextMission);

	return true;
}

void MissionBase::missionStart(void)
{
}

int MissionBase::check(void)
{
	return 0;
}

bool MissionBase::update(void)
{
	return false;
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
