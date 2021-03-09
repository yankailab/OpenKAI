/*
 * Takeoff.cpp
 *
 *  Created on: Mar 9, 2020
 *      Author: yankai
 */

#include "Takeoff.h"

namespace kai
{

Takeoff::Takeoff()
{
	m_type = state_takeoff;
	m_alt = 10.0;
}

Takeoff::~Takeoff()
{
}

bool Takeoff::init(void* pKiss)
{
	IF_F(!this->State::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("alt",&m_alt);

	return true;
}

bool Takeoff::update(void)
{
	IF_F(!this->State::update());

	LOG_I("Takeoff complete");
	return true;
}

void Takeoff::draw(void)
{
	this->State::draw();
	addMsg("alt = "+f2str(m_alt));

}

}
