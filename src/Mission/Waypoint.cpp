/*
 * Waypoint.cpp
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#include "Waypoint.h"

namespace kai
{

Waypoint::Waypoint()
{
}

Waypoint::~Waypoint()
{
}

bool Waypoint::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

//	KISSm(pK,nFIFO);

	return true;
}

int Waypoint::check(void)
{
	return 0;
}

bool Waypoint::update(void)
{
	return false;
}

bool Waypoint::draw(void)
{
	IF_F(!this->MissionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;

	return true;
}

bool Waypoint::cli(int& iY)
{
	IF_F(!this->MissionBase::cli(iY));

	string msg = "Waypoint";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
