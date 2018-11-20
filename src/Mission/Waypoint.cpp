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
	m_pMavlink = NULL;
	m_pDS = NULL;
	m_vWP.init();
	m_vPos.init();
	m_vErr.init();
	m_speedV = 1.0;
	m_speedH = 1.0;
	m_hdg = 0.0;
	m_r = 3.0;
}

Waypoint::~Waypoint()
{
}

bool Waypoint::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("lat", &m_vWP.x);
	pK->v("lng", &m_vWP.y);
	pK->v("alt", &m_vWP.z);

	KISSm(pK,speedV);
	KISSm(pK,speedH);
	KISSm(pK,hdg);
	KISSm(pK,r);

	string iName;
	iName = "";
	pK->v("_Mavlink", &iName);
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_F(m_pMavlink);

	iName = "";
	F_INFO(pK->v("_DistSensorBase", &iName));
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDS,iName + " not found");

	return true;
}

int Waypoint::check(void)
{
	IF__(!m_pMavlink,-1);

	return this->MissionBase::check();
}

bool Waypoint::update(void)
{
	IF_F(check()<0);

	m_vPos.x = ((double)m_pMavlink->m_msg.global_position_int.lat) * 0.0000001;
	m_vPos.y = ((double)m_pMavlink->m_msg.global_position_int.lon) * 0.0000001;
	if(!m_pDS)
	{
		m_vPos.z = ((double)m_pMavlink->m_msg.global_position_int.relative_alt) * 0.001;
	}
	else
	{
		m_vPos.z = m_pDS->dAvr();
		if(m_vPos.z < 0.0)
		{
			//Lidar out of range
			m_vPos.z = DBL_MAX;
		}
	}

	m_vErr = m_vWP - m_vPos;
	double d = m_vErr.len();

	//Mission complete
	IF_T(d < m_r);

	return false;
}

bool Waypoint::draw(void)
{
	IF_F(!this->MissionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	string msg;

	pWin->tabNext();

	msg = "WP = (" + f2str(m_vWP.x) + ", "
				   + f2str(m_vWP.y) + ", "
		           + f2str(m_vWP.z) + ")";
	pWin->addMsg(&msg);

	msg = "Pos = (" + f2str(m_vPos.x) + ", "
				   + f2str(m_vPos.y) + ", "
		           + f2str(m_vPos.z) + ")";
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool Waypoint::cli(int& iY)
{
	IF_F(!this->MissionBase::cli(iY));

	string msg;

	msg = "WP = (" + f2str(m_vWP.x) + ", "
				   + f2str(m_vWP.y) + ", "
		           + f2str(m_vWP.z) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Pos = (" + f2str(m_vPos.x) + ", "
				   + f2str(m_vPos.y) + ", "
		           + f2str(m_vPos.z) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
