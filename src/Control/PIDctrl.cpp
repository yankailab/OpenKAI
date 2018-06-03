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
	m_P = 0;
	m_I = 0;
	m_Imax = 0;
	m_D = 0;
	m_dT = 0;
	m_min = 0.0;
	m_max = DBL_MAX;
	m_K = 1.0;

	reset();
}

PIDctrl::~PIDctrl()
{
}

bool PIDctrl::init(void* pKiss)
{
	IF_F(!this->ControlBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, P);
	KISSm(pK, I);
	KISSm(pK, Imax);
	KISSm(pK, D);
	KISSm(pK, dT);
	KISSm(pK, min);
	KISSm(pK, max);
	KISSm(pK, K);

	return true;
}

bool PIDctrl::link(void)
{
	IF_F(!this->ControlBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	return true;
}

double PIDctrl::update(double v, double vTarget)
{
	const static double ovT = 1.0/USEC_1SEC;
	uint64_t tNow = getTimeUsec();

	m_vPred = v + (v - m_v) * ovT * m_dT;
	m_v = v;
	m_vTarget = vTarget;

	double ovDT = 1.0 / (double)(tNow - m_tLastUpdate);
	m_tLastUpdate = tNow;

	m_eOld = m_e;
	m_e = m_vTarget - m_vPred;
	m_eInteg += m_e;

	double o = m_P * m_e
			 + m_D * (m_e - m_eOld) * ovDT
			 + constrain(m_I * m_eInteg, -m_Imax, m_Imax);

	m_output = constrain(o*m_K, m_min, m_max);

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
