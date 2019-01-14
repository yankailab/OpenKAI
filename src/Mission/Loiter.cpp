/*
 * Loiter.cpp
 *
 *  Created on: Jan 14, 2019
 *      Author: yankai
 */

#include "Loiter.h"

namespace kai
{

Loiter::Loiter()
{
	m_hdg = 0;
	reset();
}

Loiter::~Loiter()
{
}

bool Loiter::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,hdg);

	return true;
}

bool Loiter::missionStart(void)
{
	IF_F(!m_bPos);

	return this->MissionBase::missionStart();
}

bool Loiter::update(void)
{
	IF_F(m_tStart <= 0);
	IF_F(m_tTimeout <= 0);

	if(getTimeUsec() > m_tStart + m_tTimeout)
	{
		LOG_I("Mission complete");
		reset();
		return true;
	}

	return false;
}

void Loiter::reset(void)
{
	m_bPos = false;
	m_vPos.init();
	this->MissionBase::reset();
}

void Loiter::setPos(vDouble3& p)
{
	m_vPos = p;
	m_bPos = true;
}

bool Loiter::draw(void)
{
	IF_F(!this->MissionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	string msg;

	pWin->tabNext();

	pWin->addMsg("Pos = (" + f2str(m_vPos.x,7) + ", "
				   + f2str(m_vPos.y,7) + ", "
		           + f2str(m_vPos.z,7) + ")");

	pWin->tabPrev();

	return true;
}

bool Loiter::console(int& iY)
{
	IF_F(!this->MissionBase::console(iY));

	string msg;

	C_MSG("Pos = (" + f2str(m_vPos.x,7) + ", "
				    + f2str(m_vPos.y,7) + ", "
		            + f2str(m_vPos.z,7) + ")");

	return true;
}

}
