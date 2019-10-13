/*
 * Move.cpp
 *
 *  Created on: March 16, 2019
 *      Author: yankai
 */

#include "Move.h"

namespace kai
{

Move::Move()
{
	m_hdg = 0;
	reset();
}

Move::~Move()
{
}

bool Move::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("hdg",&m_hdg);

	return true;
}

bool Move::update(void)
{
	this->MissionBase::update();

	IF_F(m_tTimeout <= 0);
	IF_F(m_tStamp < m_tStart + m_tTimeout);

	LOG_I("Timeout");
	return true;
}

void Move::reset(void)
{
	m_vPos.init();
	this->MissionBase::reset();
}

void Move::setPos(vDouble3& p)
{
	m_vPos = p;
}

void Move::draw(void)
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
