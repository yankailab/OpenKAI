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
		m_vMin = -FLT_MAX;
		m_vMax = FLT_MAX;
		m_oMin = -FLT_MAX;
		m_oMax = FLT_MAX;
		m_P = 0;
		m_I = 0;
		m_Imax = 0;
		m_D = 0;
		reset();
	}

	PID::~PID()
	{
	}

	bool PID::init(void *pKiss)
	{
		IF_F(!this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vMin", &m_vMin);
		pK->v("vMax", &m_vMax);
		pK->v("oMin", &m_oMin);
		pK->v("oMax", &m_oMax);

		pK->v("P", &m_P);
		pK->v("I", &m_I);
		pK->v("Imax", &m_Imax);
		pK->v("D", &m_D);

		return true;
	}

	void PID::reset(void)
	{
		m_v = 0.0;
		m_vT = 0.0;
		m_e = 0.0;
		m_eOld = 0.0;
		m_eI = 0.0;

		m_vOut = 0.0;
	}

	float PID::update(float v, float vT, float dT)
	{
		float ovdT = 0.0;
		if (dT != 0.0)
			ovdT = 1.0 / dT;

		m_v = constrain(v, m_vMin, m_vMax);
		m_vT = vT;

		m_eOld = m_e;
		m_e = m_vT - m_v;
		m_eI += m_e * dT;

		//P,I,D should be of the same symbol
		float o = m_P * m_e + m_D * (m_e - m_eOld) * ovdT + constrain(m_I * m_eI, -m_Imax, m_Imax);

		m_vOut = constrain(o, m_oMin, m_oMax);
		return m_vOut;
	}

	double PID::o(void)
	{
		return m_vOut;
	}

	void PID::console(void *pConsole)
	{
		NULL_(pConsole);
		this->BASE::console(pConsole);

		((_Console *)pConsole)->addMsg("v=" + f2str(m_v) + ", vT=" + f2str(m_vT) + ", vO=" + f2str(m_vOut), 1);
	}

}
