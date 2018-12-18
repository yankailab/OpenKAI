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
	m_tag = -1;
	m_hdg = -1.0;
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

	return true;
}

int Land::check(void)
{
	return this->MissionBase::check();
}

bool Land::update(void)
{
	IF_F(check()<0);

	return false;
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
