/*
 * Land.cpp
 *
 *  Created on: Dec 18, 2018
 *      Author: yankai
 */

#include "Land.h"

namespace kai
{

Land::Land()
{
	m_tag = 0;
	m_hdg = 0.0;
	m_speed = 1.0;
	reset();
}

Land::~Land()
{
}

bool Land::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,tag);
	KISSm(pK,hdg);
	KISSm(pK,speed);

	return true;
}

bool Land::update(void)
{
	IF_F(!m_bLanded);

	LOG_I("Landed");
	return true;
}

void Land::reset(void)
{
	m_bLanded = false;
	this->MissionBase::reset();
}

void Land::setLanded(bool bLanded)
{
	m_bLanded = bLanded;
}

bool Land::draw(void)
{
	IF_F(!this->MissionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	string msg;

	pWin->tabNext();
//	pWin->addMsg("");
	pWin->tabPrev();

	return true;
}

bool Land::console(int& iY)
{
	IF_F(!this->MissionBase::console(iY));

	string msg;
//	C_MSG("");

	return true;
}

}
