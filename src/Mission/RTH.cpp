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
	m_alt = 20.0;
	m_speedV = 1.0;
	m_speedH = 1.0;
	m_hdg = 0.0;
	m_r = 3.0;

	reset();
}

RTH::~RTH()
{
}

bool RTH::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,alt);
	KISSm(pK,speedV);
	KISSm(pK,speedH);
	KISSm(pK,hdg);
	KISSm(pK,r);

	return true;
}

bool RTH::update(void)
{
	IF_F(!m_bSetHome);

	m_vErr = m_vHome - m_vPos;
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

void RTH::reset(void)
{
	m_vErr.init();
	m_vHome.init();
	m_vPos.init();
	m_bSetHome = false;
}

void RTH::setHome(vDouble3& p)
{
	m_vHome = p;
	m_bSetHome = true;
}

void RTH::setPos(vDouble3& p)
{
	m_vPos = p;
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

	return true;
}

}
