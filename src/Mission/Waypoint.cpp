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
	m_bAlt = true;

	reset();
}

Waypoint::~Waypoint()
{
}

bool Waypoint::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,bHoffset);
	KISSm(pK,bVoffset);
	KISSm(pK,bHdgOffset);
	KISSm(pK,speedV);
	KISSm(pK,speedH);
	KISSm(pK,hdg);
	KISSm(pK,r);
	KISSm(pK,bAlt);

	pK->v("x", &m_vWP.x);	//lat/northing
	pK->v("y", &m_vWP.y);	//lng/easting
	pK->v("alt",&m_vWP.z);

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

			LL_POS pLL;
			pLL.m_lat = m_vPos.x;
			pLL.m_lng = m_vPos.y;
			pLL.m_hdg = m_hdg;

			GPS gps;
			UTM_POS pUTM = gps.LL2UTM(pLL);
			pUTM = gps.offset(pUTM, m_vWP);
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

	m_eH = dEarth(m_vWP.x, m_vWP.y, m_vPos.x, m_vPos.y);
	m_eV = abs(m_vWP.z - m_vPos.z);

	IF_F(m_eH > m_r);
	IF_T(!m_bAlt);
	IF_F(m_eV > m_r);

	LOG_I("WP complete");
	return true;
}

void Waypoint::reset(void)
{
	m_bSetWP = false;
	m_bSetPos = false;
	m_eH = 0.0;
	m_eV = 0.0;
	m_vPos.init();
	m_vWPtarget.init();
	this->MissionBase::reset();
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

	pWin->addMsg("eH = " + f2str(m_eH,7) +
				 ", eV = " + f2str(m_eV,7));

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

	C_MSG("eH = " + f2str(m_eH,7) +
		  ", eV = " + f2str(m_eV,7));

	return true;
}

}
