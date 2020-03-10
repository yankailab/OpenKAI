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
	m_type = mission_land;

	m_tag = 0;
	m_hdg = 0.0;
	m_speed = 1.0;
}

Land::~Land()
{
}

bool Land::init(void* pKiss)
{
	IF_F(!this->MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("tag",&m_tag);
	pK->v("hdg",&m_hdg);
	pK->v("speed",&m_speed);

	Kiss* pT = pK->o("tag");
	IF_T(pT->empty());
	Kiss** ppT = pT->getChildItr();

	LAND_TAG t;
	unsigned int i = 0;
	while (ppT[i])
	{
		pT = ppT[i++];

		t.init();
		pT->v("iTag",&t.m_iTag);
		pT->v("angle",&t.m_angle);

		m_vTag.push_back(t);
	}

	return true;
}

bool Land::update(void)
{
	IF_F(!this->MissionBase::update());

	LOG_I("Landed");
	return true;
}

void Land::draw(void)
{
	this->MissionBase::draw();
}

}
