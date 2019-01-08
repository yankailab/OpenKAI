/*
 * PIDctrl.cpp
 *
 *  Created on: May 18, 2015
 *      Author: yankai
 */

#include "PIDctrl.h"

namespace kai
{

PIDctrl::PIDctrl()
{
	m_dT = 0;
	m_min = 0.0;
	m_max = DBL_MAX;
	m_K = 1.0;
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

	KISSm(pK, dT);
	KISSm(pK, min);
	KISSm(pK, max);
	KISSm(pK, K);

	KISSm(pK, P);
	KISSm(pK, I);
	KISSm(pK, Imax);
	KISSm(pK, D);

	return true;
}

double PIDctrl::update(double v, double vTarget)
{
	uint64_t tNow = getTimeUsec();

	m_vPred = v + (v - m_v) * m_dT;
	m_v = v;
	m_vTarget = vTarget;

	m_eOld = m_e;
	m_e = m_vTarget - m_vPred;
	m_eInteg += m_e;

	//P,I,D should be of the same symbol
	double o = m_P * m_e
			 + m_D * (m_e - m_eOld) * (USEC_1SEC / (double)(tNow - m_tLastUpdate)) // unit: sec
			 + constrain(m_I * m_eInteg, -m_Imax, m_Imax);

	m_output = constrain(o*m_K, m_min, m_max);

	m_tLastUpdate = tNow;
	return m_output;
}

double PIDctrl::o(void)
{
	return m_output;
}

bool PIDctrl::draw(void)
{
	return this->ControlBase::draw();
}

bool PIDctrl::console(int& iY)
{
	IF_F(!this->ControlBase::console(iY));

	string msg;
	msg = "v=" + f2str(m_v)
			+ " vPred=" + f2str(m_vPred)
			+ " vTarget=" + f2str(m_vTarget);
			+ " output=" + f2str(m_output);

	COL_MSG;
	iY++;
	mvaddstr(iY, CONSOLE_X_MSG, msg.c_str());

	return true;
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

}
