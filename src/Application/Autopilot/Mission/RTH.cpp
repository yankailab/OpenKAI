/*
 * RTH.cpp
 *
 *  Created on: Jan 14, 2019
 *      Author: yankai
 */

#include "RTH.h"

namespace kai
{

RTH::RTH()
{
	m_speedV = 1.0;
	m_speedH = 1.0;
	m_hdg = 0.0;
	m_r = 3.0;
	m_vHome.init();
	m_vHome.z = 20.0;
	m_bAlt = true;

	reset();
}

RTH::~RTH()
{
}

bool RTH::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("speedV",&m_speedV);
	pK->v("speedH",&m_speedH);
	pK->v("hdg",&m_hdg);
	pK->v("r",&m_r);
	pK->v("bAlt",&m_bAlt);
	pK->v("alt",&m_vHome.z);

	return true;
}

bool RTH::update(void)
{
	IF_F(!m_bSetHome);

	vDouble2 dH;
	dH.x = m_vPos.x - m_vHome.x;
	dH.y = m_vPos.y - m_vHome.y;
	m_eH = dH.len() * 1e5;	//1deg ~= 10^5m
	m_eV = abs(m_vHome.z - m_vPos.z);

	IF_F(m_eH > m_r);
	IF_T(!m_bAlt);
	IF_F(m_eV > m_r);

	LOG_I("RTH complete");
	return true;
}

void RTH::reset(void)
{
	m_eH = 0.0;
	m_eV = 0.0;
	m_vHome.init();
	m_vPos.init();
	m_bSetHome = false;
	this->MissionBase::reset();
}

void RTH::setHome(vDouble3& p)
{
	m_vHome.x = p.x;
	m_vHome.y = p.y;
	m_bSetHome = true;
}

void RTH::setPos(vDouble3& p)
{
	m_vPos = p;
}

vDouble3 RTH::getHome(void)
{
	return m_vHome;
}

double RTH::getHdg(void)
{
	return m_hdg;
}

bool RTH::draw(void)
{
	IF_F(!this->MissionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	string msg;

	pWin->tabNext();

	pWin->addMsg("Home = (" + f2str(m_vHome.x,7) + ", "
				   + f2str(m_vHome.y,7) + ", "
		           + f2str(m_vHome.z,7) + ")");

	pWin->addMsg("Pos = (" + f2str(m_vPos.x,7) + ", "
				   + f2str(m_vPos.y,7) + ", "
		           + f2str(m_vPos.z,7) + ")");

	pWin->addMsg("eH = " + f2str(m_eH,7) +
				 ", eV = " + f2str(m_eV,7));

	pWin->tabPrev();

	return true;
}

bool RTH::console(int& iY)
{
	IF_F(!this->MissionBase::console(iY));

	string msg;

	C_MSG("Home = (" + f2str(m_vHome.x,7) + ", "
				   + f2str(m_vHome.y,7) + ", "
		           + f2str(m_vHome.z,7) + ")");

	C_MSG("Pos = (" + f2str(m_vPos.x,7) + ", "
				    + f2str(m_vPos.y,7) + ", "
		            + f2str(m_vPos.z,7) + ")");

	C_MSG("eH = " + f2str(m_eH,7) +
		  ", eV = " + f2str(m_eV,7));

	return true;
}

}
