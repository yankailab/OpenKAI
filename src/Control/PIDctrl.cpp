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

	resetState();
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

	return true;
}

bool PIDctrl::link(void)
{
	IF_F(!this->ControlBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	return true;
}

void PIDctrl::update(void)
{

}

bool PIDctrl::draw(void)
{
	return this->ControlBase::draw();
}

bool PIDctrl::cli(int& iY)
{
	IF_F(!this->ControlBase::cli(iY));

	return true;
}

void PIDctrl::reset(void)
{

}

void PIDctrl::resetState(void)
{
	m_v = 0.0;
	m_vPred = 0.0;
	m_vTarget = 0.0;

	m_e = 0.0;
	m_eOld = 0.0;
	m_eInteg = 0.0;
}

}
