/*
 * commonAutopilot.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_
#define OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_

#include "../Interface/_Mavlink.h"
#include "../Interface/_RC.h"

#define NUM_RC_CHANNEL 8

namespace kai
{

struct CONTROL_PID
{
	double m_P;
	double m_I;
	double m_Imax;
	double m_D;
	double m_dT;

	void reset(void)
	{
		m_P = 0;
		m_I = 0;
		m_Imax = 0;
		m_D = 0;
		m_dT = 0;
	}
};

struct RC_CHANNEL
{
	int m_pwm;
	int m_pwmL;
	int m_pwmN;
	int m_pwmH;
	int m_iCh;

	void reset(void)
	{
		m_pwmL = 1000;
		m_pwmN = 1500;
		m_pwmH = 2000;
		m_pwm = m_pwmN;
		m_iCh = 0;
	}

	void neutral(void)
	{
		m_pwm = m_pwmN;
	}
};

struct CONTROL_CHANNEL
{
	double m_pos;
	double m_targetPos;

	double m_err;
	double m_errOld;
	double m_errInteg;

	void resetErr(void)
	{
		m_err = 0;
		m_errOld = 0;
		m_errInteg = 0;
	}

	void reset(void)
	{
		resetErr();
		m_pos = 0;
		m_targetPos = 0;
	}
};

struct CONTROL_PARAM
{
	CONTROL_PID m_pid;
	RC_CHANNEL m_RC;
};

struct AUTOPILOT_CONTROL
{
	_RC* m_pRC;
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	CONTROL_PARAM m_roll;
	CONTROL_PARAM m_pitch;
	CONTROL_PARAM m_yaw;
	CONTROL_PARAM m_alt;

	void reset(void)
	{
		m_pRC = NULL;
		m_pMavlink = NULL;
		m_lastHeartbeat = 0;
		m_iHeartbeat = 0;
	}

	void RCneutral(void)
	{
		m_roll.m_RC.neutral();
		m_pitch.m_RC.neutral();
		m_yaw.m_RC.neutral();
		m_alt.m_RC.neutral();
	}
};

}

#endif /* OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_ */
