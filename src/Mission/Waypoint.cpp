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
	m_vWP.init();
	m_vSpeed.init();
	m_yaw = 0.0;
	m_yawSpeed = 180.0;
	m_r = 3.0;
}

Waypoint::~Waypoint()
{
}

bool Waypoint::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,r);
	KISSm(pK,yaw);
	KISSm(pK,yawSpeed);

	pK->v("lat", &m_vWP.x);
	pK->v("lng", &m_vWP.y);
	pK->v("alt", &m_vWP.z);

	pK->v("vVertical", &m_vSpeed.x);
	pK->v("vHorizontal", &m_vSpeed.y);

	string iName;
	iName = "";
	pK->v("_Mavlink", &iName);
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_F(m_pMavlink);

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

	m_vD.x = ((double)m_pMavlink->m_msg.global_position_int.lat) * 0.0000001;
	m_vD.y = ((double)m_pMavlink->m_msg.global_position_int.lon) * 0.0000001;
	m_vD.z = ((double)m_pMavlink->m_msg.global_position_int.relative_alt) * 0.001;

	vDouble3 dPos = m_vD - m_vWP;
	double d = dPos.len();

	if(d < m_r)
	{
		//Mission complete
		return true;
	}

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
