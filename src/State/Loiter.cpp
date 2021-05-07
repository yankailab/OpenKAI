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
	m_type = state_loiter;
	m_hdg = 0;
	reset();
}

Loiter::~Loiter()
{
}

bool Loiter::init(void* pKiss)
{
	IF_F(!this->State::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("hdg",&m_hdg);

	return true;
}

bool Loiter::update(void)
{
	return this->State::update();
}

void Loiter::reset(void)
{
	m_vPos.init();
	this->State::reset();
}

void Loiter::setPos(vDouble3& p)
{
	m_vPos = p;
}

void Loiter::console(void* pConsole)
{
	NULL_(pConsole);
	this->State::console(pConsole);

	_Console *pC = (_Console *)pConsole;
	pC->addMsg("Pos = (" + f2str(m_vPos.x,7) + ", "
				   + f2str(m_vPos.y,7) + ", "
		           + f2str(m_vPos.z,7) + ")",1);

	int tOut = (int)((double)(m_tTimeout - (m_tStamp - m_tStart))*USEC_2_SEC);
	tOut = constrain(tOut, 0, INT_MAX);
	pC->addMsg("Timeout = " + i2str(tOut),1);
}

}
