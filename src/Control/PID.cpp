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
		m_P = 0;
		m_I = 0;
		m_Imax = 0;
		m_D = 0;
		m_vRin.set(-FLT_MAX, FLT_MAX);
		m_vRout.set(-FLT_MAX, FLT_MAX);

		reset();
	}

	PID::~PID()
	{
	}

	int PID::init(void *pKiss)
	{
		CHECK_(this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("P", &m_P);
		pK->v("I", &m_I);
		pK->v("Imax", &m_Imax);
		pK->v("D", &m_D);

		pK->v("vRin", &m_vRin);
		pK->v("vRout", &m_vRout);

		return OK_OK;
	}

	void PID::reset(void)
	{
		m_vVar = 0.0;
		m_vSetPoint = 0.0;
		m_e = 0.0;
		m_eOld = 0.0;
		m_eI = 0.0;

		m_vOut = 0.0;
	}

	float PID::update(float v, float sp, float dT)
	{
		float ovdT = 0.0;
		if (dT != 0.0)
			ovdT = 1.0 / dT;

		m_vVar = m_vRin.constrain(v);
		m_vSetPoint = sp;

		m_eOld = m_e;
		m_e = m_vSetPoint - m_vVar;
		m_eI += m_e * dT;

		// P,I,D should be of the same symbol
		float o = m_P * m_e + m_D * (m_e - m_eOld) * ovdT + constrain(m_I * m_eI, -m_Imax, m_Imax);

		m_vOut = m_vRout.constrain(o);
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

		((_Console *)pConsole)->addMsg("v=" + f2str(m_vVar) + ", sp=" + f2str(m_vSetPoint) + ", vO=" + f2str(m_vOut), 1);
	}

}
