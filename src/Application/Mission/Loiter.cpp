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

	pK->v("hdg",&m_hdg);

	return true;
}

bool Loiter::update(void)
{
	this->MissionBase::update();

	IF_F(m_tTimeout <= 0);
	IF_F(m_tStamp < m_tStart + m_tTimeout);

	LOG_I("Timeout");
	return true;
}

void Loiter::reset(void)
{
	m_vPos.init();
	this->MissionBase::reset();
}

void Loiter::setPos(vDouble3& p)
{
	m_vPos = p;
}

void Loiter::draw(void)
{
	this->MissionBase::draw();

	addMsg("Pos = (" + f2str(m_vPos.x,7) + ", "
				   + f2str(m_vPos.y,7) + ", "
		           + f2str(m_vPos.z,7) + ")",1);

	int tOut = (int)((double)(m_tTimeout - (m_tStamp - m_tStart))*OV_USEC_1SEC);
	tOut = constrain(tOut, 0, INT_MAX);
	addMsg("Timeout = " + i2str(tOut),1);
}

}
