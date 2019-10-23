/*
 * PIDctrl.cpp
 *
 *  Created on: May 18, 2018
 *      Author: yankai
 */

#include "PIDctrl.h"

namespace kai
{

PIDctrl::PIDctrl()
{
	m_dT = 0;
	m_oMin = -FLT_MAX;
	m_oMax = FLT_MAX;
	m_vMin = -FLT_MAX;
	m_vMax = FLT_MAX;
	m_P = 0;
	m_I = 0;
	m_Imax = 0;
	m_D = 0;
	reset();
}

PIDctrl::~PIDctrl()
{
}

bool PIDctrl::init(void* pKiss)
{
	IF_F(!this->ControlBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vMin",&m_vMin);
	pK->v("vMax",&m_vMax);
	pK->v("dT",&m_dT);
	pK->v("oMin",&m_oMin);
	pK->v("oMax",&m_oMax);

	pK->v("P",&m_P);
	pK->v("I",&m_I);
	pK->v("Imax",&m_Imax);
	pK->v("D",&m_D);

	return true;
}

float PIDctrl::update(float v, float vTarget, uint64_t t)
{
	if(t <= m_tLastUpdate)
		return m_output;

	if(v < m_vMin || v > m_vMax || vTarget < m_vMin || vTarget > m_vMax)
	{
		reset();
		return 0;
	}

	m_vPred = v + (v - m_v) * m_dT;
	m_v = v;
	m_vTarget = vTarget;

	m_eOld = m_e;
	m_e = m_vTarget - m_vPred;
	m_eInteg += m_e;

	//P,I,D should be of the same symbol
	float o = m_P * m_e
			 + m_D * (m_e - m_eOld) * (USEC_1SEC / (float)(t - m_tLastUpdate)) // unit: sec
			 + constrain(m_I * m_eInteg, -m_Imax, m_Imax);

	m_output = constrain(o, m_oMin, m_oMax);

	m_tLastUpdate = t;
	return m_output;
}

double PIDctrl::o(void)
{
	return m_output;
}

void PIDctrl::reset(void)
{
	m_v = 0.0;
	m_vPred = 0.0;
	m_vTarget = 0.0;

	m_e = 0.0;
	m_eOld = 0.0;
	m_eInteg = 0.0;

	m_tLastUpdate = 0;
}

void PIDctrl::draw(void)
{
	this->ControlBase::draw();

	addMsg("v=" + f2str(m_v)
			+ " vPred=" + f2str(m_vPred)
			+ " vTarget=" + f2str(m_vTarget)
			+ " vOutput=" + f2str(m_output));
}

}
