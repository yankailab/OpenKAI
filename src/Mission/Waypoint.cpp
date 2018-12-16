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
	m_dSensor = -1.0;
	m_bOffset = false;
	m_bHdgOffset = false;
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

	KISSm(pK,bOffset);
	KISSm(pK,bHdgOffset);
	KISSm(pK,speedV);
	KISSm(pK,speedH);
	KISSm(pK,hdg);
	KISSm(pK,r);

	string iName;
	iName = "";
	pK->v("_Mavlink", &iName);
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMavlink, iName + " not found");

	iName = "";
	pK->v("_DistSensorBase", &iName);
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInst(iName));
	NULLl(m_pDS, iName + " not found");

	return true;
}

void Waypoint::missionStart(void)
{
	IF_(check()<0);
	updatePos();
	IF_(!m_bOffset);

	if(m_bHdgOffset)
	{
		m_hdg += ((double)m_pMavlink->m_msg.global_position_int.hdg) * 0.01;
		m_hdg = Hdg(m_hdg);
	}

	GPS gps;
	LL_POS pLL;
	pLL.m_lat = m_vPos.x;
	pLL.m_lng = m_vPos.y;
	pLL.m_hdg = m_hdg;
	gps.update(pLL);

	UTM_POS pUTM = gps.getPos(m_vWP);
	pLL = gps.UTM2LL(pUTM);
	m_vWP.x = pLL.m_lat;
	m_vWP.y = pLL.m_lng;
	m_vWP.z = pLL.m_altRel;
}

int Waypoint::check(void)
{
	IF__(!m_pMavlink,-1);

	return this->MissionBase::check();
}

bool Waypoint::update(void)
{
	IF_F(check()<0);

	updatePos();

	m_vErr = m_vWP - m_vPos;
	m_vErr.z = 0.0;
	double d = m_vErr.len();

	if(d < m_r)
	{
		LOG_I("Mission complete");
		return true;
	}

	return false;
}

void Waypoint::updatePos(void)
{
	m_vPos.x = ((double)m_pMavlink->m_msg.global_position_int.lat) * 1e-7;
	m_vPos.y = ((double)m_pMavlink->m_msg.global_position_int.lon) * 1e-7;
	m_vPos.z = ((double)m_pMavlink->m_msg.global_position_int.relative_alt) * 1e-3;
	if(m_pDS)
	{
		m_dSensor = m_pDS->dAvr();
	}
}

bool Waypoint::draw(void)
{
	IF_F(!this->MissionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	string msg;

	pWin->tabNext();

	pWin->addMsg("WP = (" + f2str(m_vWP.x,7) + ", "
				   + f2str(m_vWP.y,7) + ", "
		           + f2str(m_vWP.z,7) + ")");

	pWin->addMsg("Pos = (" + f2str(m_vPos.x,7) + ", "
				   + f2str(m_vPos.y,7) + ", "
		           + f2str(m_vPos.z,7) + ")");

	pWin->addMsg("dSensor = " + f2str(m_dSensor));

	pWin->tabPrev();

	return true;
}

bool Waypoint::console(int& iY)
{
	IF_F(!this->MissionBase::console(iY));

	string msg;

	C_MSG("WP = (" + f2str(m_vWP.x,7) + ", "
				   + f2str(m_vWP.y,7) + ", "
		           + f2str(m_vWP.z,7) + ")");

	C_MSG("Pos = (" + f2str(m_vPos.x,7) + ", "
				    + f2str(m_vPos.y,7) + ", "
		            + f2str(m_vPos.z,7) + ")");

	C_MSG("dSensor = " + f2str(m_dSensor));

	return true;
}

}
