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
	m_type = state_land;

	m_tag = 0;
	m_hdg = 0.0;
	m_speed = 1.0;
}

Land::~Land()
{
}

bool Land::init(void* pKiss)
{
	IF_F(!this->State::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("tag",&m_tag);
	pK->v("hdg",&m_hdg);
	pK->v("speed",&m_speed);

	Kiss* pT = pK->child("tag");
	IF_T(pT->empty());

	LAND_TAG t;
	Kiss* pC;
	int i = 0;
	while (1)
	{
		pC = pT->child(i++);
		if(pC->empty())break;

		t.init();
		pC->v("iTag",&t.m_iTag);
		pC->v("angle",&t.m_angle);
		m_vTag.push_back(t);
	}

	return true;
}

bool Land::update(void)
{
	IF_F(!this->State::update());

	LOG_I("Landed");
	return true;
}

void Land::draw(void)
{
	this->State::draw();
}

}
