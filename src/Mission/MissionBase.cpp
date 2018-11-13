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
}

MissionBase::~MissionBase()
{
}

bool MissionBase::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

//	KISSm(pK,nFIFO);

	return true;
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

bool MissionBase::cli(int& iY)
{
	IF_F(!this->BASE::cli(iY));

	return true;
}

}
