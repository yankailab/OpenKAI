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

	bool PID::init(const json& j)
	{
		IF_F(!this->BASE::init(j));

		m_P = j.value("P", "");
		m_I = j.value("I", "");
		m_Imax = j.value("Imax", "");
		m_D = j.value("D", "");

		m_vRin = j.value("vRin", "");
		m_vRout = j.value("vRout", "");

		return true;
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

	float PID::getP(void)
	{
		return m_P;
	}

	void PID::setP(float P)
	{
		m_P = P;
	}

	float PID::getI(void)
	{
		return m_I;
	}

	void PID::setI(float I)
	{
		m_I = I;
	}

	float PID::getImax(void)
	{
		return m_Imax;
	}

	void PID::setImax(float Imax)
	{
		m_Imax = Imax;
	}

	float PID::getD(void)
	{
		return m_D;
	}

	void PID::setD(float D)
	{
		m_D = D;
	}

	vFloat2 PID::getRangeIn(void)
	{
		return m_vRin;
	}

	void PID::setRangeIn(const vFloat2 &vRin)
	{
		m_vRin = vRin;
	}

	vFloat2 PID::getRangeOut(void)
	{
		return m_vRout;
	}

	void PID::setRangeOut(const vFloat2 &vRout)
	{
		m_vRout = vRout;
	}

	void PID::console(void *pConsole)
	{
		NULL_(pConsole);
		this->BASE::console(pConsole);

		((_Console *)pConsole)->addMsg("P=" + f2str(m_P) + ", I=" + f2str(m_I) + ", Imax=" + f2str(m_Imax) + ", D=" + f2str(m_D), 1);
		((_Console *)pConsole)->addMsg("v=" + f2str(m_vVar) + ", sp=" + f2str(m_vSetPoint) + ", vO=" + f2str(m_vOut), 1);
	}

}
