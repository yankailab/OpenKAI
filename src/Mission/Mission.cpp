/*
 * Mission.cpp
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#include "Mission.h"

namespace kai
{

Mission::Mission()
{
	m_type = mission_base;
	m_next = "";
	m_tStamp = 0;
	m_tTimeout = 0;

	reset();
}

Mission::~Mission()
{
}

bool Mission::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("next", &m_next);
	if(pK->v("tTimeout",&m_tTimeout))
		m_tTimeout *= USEC_1SEC;

	string name;

	name = "";
	pK->parent()->parent()->v("Console", &name);
	m_pConsole = (Console*)(pK->getInst(name));

#ifdef USE_OPENCV
	name = "";
	pK->parent()->parent()->v("Window",&name);
	m_pWindow = (Window*)(pK->getInst(name));
#endif

	return true;
}

void Mission::reset(void)
{
	m_tStart = 0;
	m_bComplete = false;
}

bool Mission::update(void)
{
	if(m_bComplete)
	{
		reset();
		return true;
	}

	m_tStamp = getTimeUsec();
	if(m_tStart <= 0)
		m_tStart = m_tStamp;

	IF_F(m_tTimeout <= 0);
	IF_F(m_tStamp < m_tStart + m_tTimeout);

	LOG_I("Timeout");
	return true;
}

void Mission::complete(void)
{
	m_bComplete = true;
}

MISSION_TYPE Mission::type(void)
{
	return m_type;
}

void Mission::draw(void)
{
	this->BASE::draw();
}

}
