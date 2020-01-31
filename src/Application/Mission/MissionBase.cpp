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
	m_type = mission_base;
	m_nextMission = "";
	m_tStamp = 0;
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

	pK->v("nextMission", &m_nextMission);

	if(pK->v("tTimeout",&m_tTimeout))
		m_tTimeout *= USEC_1SEC;

	return true;
}

void MissionBase::reset(void)
{
	m_tStart = 0;
	m_bComplete = false;
}

bool MissionBase::update(void)
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

void MissionBase::complete(void)
{
	m_bComplete = true;
}

MISSION_TYPE MissionBase::type(void)
{
	return m_type;
}

void MissionBase::draw(void)
{
	this->BASE::draw();
}

}
