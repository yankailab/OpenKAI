/*
 * PIDctrl.cpp
 *
 *  Created on: May 18, 2018
 *      Author: yankai
 */

#include "PID.h"

namespace kai
{

PID::PID()
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
	m_A = 0;
	m_thrReset = FLT_MAX;
	reset();
}

PID::~PID()
{
}

bool PID::init(void* pKiss)
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

	pK->v("A", &m_A);
	pK->v("thrReset", &m_thrReset);

	return true;
}

float PID::update(float v, float vTarget, uint64_t t)
{
	if(t <= m_tLastUpdate)
		return m_output;

	if(v < m_vMin || v > m_vMax || vTarget < m_vMin || vTarget > m_vMax)
	{
		reset();
		return 0;
	}

	if(abs(vTarget - m_vTarget) > m_thrReset)
	{
		reset();
		m_v = v;
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

	m_output = constrain(m_A + o, m_oMin, m_oMax);

	m_tLastUpdate = t;
	return m_output;
}

double PID::o(void)
{
	return m_output;
}

void PID::reset(void)
{
	m_v = 0.0;
	m_vPred = 0.0;
	m_vTarget = 0.0;

	m_e = 0.0;
	m_eOld = 0.0;
	m_eInteg = 0.0;

	m_output = 0.0;
	m_tLastUpdate = 0;
}

void PID::draw(void)
{
	this->ControlBase::draw();

	addMsg("v=" + f2str(m_v)
			+ " vPred=" + f2str(m_vPred)
			+ " vTarget=" + f2str(m_vTarget)
			+ " vOutput=" + f2str(m_output), 1);
}

}
