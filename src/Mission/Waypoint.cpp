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
	m_vWP.init();
	m_speedV = 1.0;
	m_speedH = 1.0;
	m_hdg = 0.0;
	m_r = 3.0;

	m_bHoffset = false;
	m_bVoffset = false;
	m_bHdgOffset = false;

	reset();
}

Waypoint::~Waypoint()
{
}

bool Waypoint::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("x", &m_vWP.x);	//lat/northing
	pK->v("y", &m_vWP.y);	//lng/easting
	pK->v("z", &m_vWP.z);	//alt

	KISSm(pK,bHoffset);
	KISSm(pK,bVoffset);
	KISSm(pK,bHdgOffset);
	KISSm(pK,speedV);
	KISSm(pK,speedH);
	KISSm(pK,hdg);
	KISSm(pK,r);

	return true;
}

bool Waypoint::update(void)
{
	if(!m_bSetWP)
	{
		m_vWPtarget = m_vWP;

		if(m_bHoffset)
		{
			IF_F(!m_bSetPos);

			GPS gps;
			LL_POS pLL;
			pLL.m_lat = m_vPos.x;
			pLL.m_lng = m_vPos.y;
			pLL.m_hdg = m_hdg;
			gps.update(pLL);

			UTM_POS pUTM = gps.getPos(m_vWP);
			pLL = gps.UTM2LL(pUTM);

			m_vWPtarget.x = pLL.m_lat;
			m_vWPtarget.y = pLL.m_lng;
		}

		if(m_bVoffset)
		{
			IF_F(!m_bSetPos);
			m_vWPtarget.z += m_vPos.z;
		}

		m_bSetWP = true;
	}

	m_vErr = m_vWP - m_vPos;
	double d = m_vErr.len();
	if(d < m_r)
	{
		LOG_I("Mission complete");
		reset();
		return true;
	}

	LOG_I("Err = " + f2str(d));

	return false;
}

void Waypoint::reset(void)
{
	m_bSetWP = false;
	m_bSetPos = false;
	m_vErr.init();
	m_vPos.init();
	m_vWPtarget.init();
	m_vErr.init();
}

void Waypoint::setPos(vDouble3& p)
{
	m_vPos = p;
	m_bSetPos = true;
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

	return true;
}

}
