/*
 * commonAutopilot.h
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_
#define OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_

namespace kai
{

struct CONTROL_PID
{
	double m_P;
	double m_I;
	double m_Imax;
	double m_D;
	double m_dT;
};

struct RC_CHANNEL
{
	int m_pwm;
	int m_pwmL;
	int m_pwmN;
	int m_pwmH;
	int m_iCh;
};

struct CONTROL_CHANNEL
{
	double m_pos;
	double m_targetPos;

	double m_err;
	double m_errOld;
	double m_errInteg;

	CONTROL_PID m_pid;
	RC_CHANNEL m_RC;

	void reset(void)
	{
		m_pos = 0;
		m_err = 0;
		m_errOld = 0;
		m_errInteg = 0;

		//m_targetPos = 0;
	}
};

}

#endif /* OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_ */
