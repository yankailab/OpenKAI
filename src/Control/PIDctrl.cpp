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
	m_pid1.init();
	m_pid2.init();

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

	Kiss* pP = NULL;

	pP = pK->o("PIDset1");
	NULL_F(pP);
	pP->v("P", &m_pid1.m_P);
	pP->v("I", &m_pid1.m_I);
	pP->v("Imax", &m_pid1.m_Imax);
	pP->v("D", &m_pid1.m_D);
	pP->v("h", &m_pid1.m_h);

	pP = pK->o("PIDset2");
	if(!pP)
	{
		m_pid2 = m_pid1;
	}
	else
	{
		pP->v("P", &m_pid2.m_P);
		pP->v("I", &m_pid2.m_I);
		pP->v("Imax", &m_pid2.m_Imax);
		pP->v("D", &m_pid2.m_D);
		pP->v("h", &m_pid2.m_h);
	}

	if(m_pid1.m_h == m_pid2.m_h)
		m_ovdH = 0.0;
	else
		m_ovdH = 1.0/abs(m_pid2.m_h - m_pid1.m_h);
	m_dP = m_pid2.m_P - m_pid1.m_P;
	m_dI = m_pid2.m_I - m_pid1.m_I;
	m_dImax = m_pid2.m_Imax - m_pid1.m_Imax;
	m_dD = m_pid2.m_D - m_pid1.m_D;

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

	//PID should be of the same symbol
	double o = m_pid1.m_P * m_e
			 + m_pid1.m_D * (m_e - m_eOld) * (USEC_1SEC / (double)(tNow - m_tLastUpdate)) // unit: sec
			 + constrain(m_pid1.m_I * m_eInteg, -m_pid1.m_Imax, m_pid1.m_Imax);

	m_output = constrain(o*m_K, m_min, m_max);

	m_tLastUpdate = tNow;
	return m_output;
}

double PIDctrl::update(double v, double vTarget, double h)
{
	uint64_t tNow = getTimeUsec();

	m_vPred = v + (v - m_v) * m_dT;
	m_v = v;
	m_vTarget = vTarget;

	m_eOld = m_e;
	m_e = m_vTarget - m_vPred;
	m_eInteg += m_e;

	h = constrain(h, m_pid1.m_h, m_pid2.m_h);
	h = (h - m_pid1.m_h)*m_ovdH;

	double P = m_pid1.m_P + h*m_dP;
	double D = m_pid1.m_D + h*m_dD;
	double I = m_pid1.m_I + h*m_dI;
	double Imax = m_pid1.m_Imax + h*m_dImax;

	//PID should be of the same symbol
	double o = P * m_e
			 + D * (m_e - m_eOld) * (USEC_1SEC / (double)(tNow - m_tLastUpdate)) // unit: sec
			 + constrain(I * m_eInteg, -Imax, Imax);

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

bool PIDctrl::cli(int& iY)
{
	IF_F(!this->ControlBase::cli(iY));

	string msg;
	msg = "v=" + f2str(m_v)
			+ " vPred=" + f2str(m_vPred)
			+ " vTarget=" + f2str(m_vTarget);
			+ " output=" + f2str(m_output);

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

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
