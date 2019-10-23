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
	m_iWP = 0;
	m_dWP = 1;
	m_loop = wp_loop_none,
	m_vPos.init(-1.0);
	m_vErr.init(-1.0);

	m_type = mission_wp;
}

Waypoint::~Waypoint()
{
}

bool Waypoint::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("loop",(int*)&m_loop);

	Kiss** ppP = pK->getChildItr();
	int i = 0;
	while (ppP[i])
	{
		Kiss* pP = ppP[i++];

		MISSION_WAYPOINT p;
		p.init();
		pP->v("vP",&p.m_vP);
		pP->v("vV",&p.m_vV);
		pP->v("vErr",&p.m_vErr);

		m_vWP.push_back(p);
	}

	return true;
}

int Waypoint::check(void)
{
	IF__(m_iWP < 0, -1);
	IF__(m_iWP >= m_vWP.size(), -1);
	IF__(m_vWP.empty(), -1);

	return 0;
}

void Waypoint::setPos(vDouble4& vPos)
{
	m_vPos = vPos;
}

MISSION_WAYPOINT* Waypoint::getWaypoint(void)
{
	IF__(check()<0, NULL);

	return &m_vWP[m_iWP];
}

MISSION_WAYPOINT* Waypoint::getClosestWaypoint(void)
{
	IF__(check()<0, NULL);

	double minD = DBL_MAX;
	MISSION_WAYPOINT* pMin = NULL;

	for(int i=0;i<m_vWP.size();i++)
	{
		MISSION_WAYPOINT* pWP = &m_vWP[i];
		double d = abs(pWP->m_vP.x - m_vPos.x) + abs(pWP->m_vP.y - m_vPos.y);
		IF_CONT(d > minD);

		minD = d;
		pMin = pWP;
	}

	return pMin;
}

bool Waypoint::update(void)
{
	IF_F(check()<0);

	MISSION_WAYPOINT* pWP = &m_vWP[m_iWP];
	IF_F(!pWP->update(m_vPos, &m_vErr));

	LOG_I("WP: " + i2str(m_iWP) +" complete");

	m_iWP += m_dWP;

	if(m_iWP < 0)
	{
		IF_T(m_loop == wp_loop_none);

		if(m_loop == wp_loop_repeat)
		{
			m_iWP = m_vWP.size()-1;
		}
		else if(m_loop == wp_loop_shuttle)
		{
			m_iWP = 0;
			m_dWP *= -1;
		}
	}
	else if(m_iWP >= m_vWP.size())
	{
		IF_T(m_loop == wp_loop_none);

		if(m_loop == wp_loop_repeat)
		{
			m_iWP = 0;
		}
		else if(m_loop == wp_loop_shuttle)
		{
			m_iWP = m_vWP.size()-1;
			m_dWP *= -1;
		}
	}

	return false;
}

void Waypoint::reset(void)
{
	this->MissionBase::reset();

	m_iWP = 0;
}

void Waypoint::draw(void)
{
	this->MissionBase::draw();
	IF_(check()<0);

	string msg;
	MISSION_WAYPOINT* pWP = &m_vWP[m_iWP];

	addMsg("nWP=" + i2str(m_vWP.size()) + ", iWP=" + i2str(m_iWP),1);

	addMsg("WP = (" + f2str(pWP->m_vP.x,7) + ", "
				   + f2str(pWP->m_vP.y,7) + ", "
				   + f2str(pWP->m_vP.x,7) + ", "
		           + f2str(pWP->m_vP.w,7) + ")",1);

	addMsg("Pos = (" + f2str(m_vPos.x,7) + ", "
				   + f2str(m_vPos.y,7) + ", "
				   + f2str(m_vPos.z,7) + ", "
		           + f2str(m_vPos.w,7) + ")",1);

	addMsg("Err = (" + f2str(m_vErr.x,7) + ", "
				   + f2str(m_vErr.y,7) + ", "
				   + f2str(m_vErr.z,7) + ", "
		           + f2str(m_vErr.w,7) + ")",1);

}

}
