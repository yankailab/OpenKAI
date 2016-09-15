/*
 * commonAutopilot.h
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_
#define OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_

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

struct AUTOPILOT_CONTROL
{
	CONTROL_CHANNEL m_roll;
	CONTROL_CHANNEL m_pitch;
	CONTROL_CHANNEL m_yaw;
	CONTROL_CHANNEL m_alt;

	int m_nRC;
	int m_pRC[NUM_RC_CHANNEL];

	void reset(void)
	{
		m_roll.reset();
		m_pitch.reset();
		m_yaw.reset();
		m_alt.reset();

		m_pRC[m_roll.m_RC.m_iCh] = m_roll.m_RC.m_pwmN;
		m_pRC[m_pitch.m_RC.m_iCh] = m_pitch.m_RC.m_pwmN;
		m_pRC[m_yaw.m_RC.m_iCh] = m_yaw.m_RC.m_pwmN;
		m_pRC[m_alt.m_RC.m_iCh] = m_alt.m_RC.m_pwmN;
	}

	void resetErr(void)
	{
		m_roll.resetErr();
		m_pitch.resetErr();
		m_yaw.resetErr();
		m_alt.resetErr();
	}

	void resetRC(void)
	{
		m_pRC[m_roll.m_RC.m_iCh] = m_roll.m_RC.m_pwmN;
		m_pRC[m_pitch.m_RC.m_iCh] = m_pitch.m_RC.m_pwmN;
		m_pRC[m_yaw.m_RC.m_iCh] = m_yaw.m_RC.m_pwmN;
		m_pRC[m_alt.m_RC.m_iCh] = m_alt.m_RC.m_pwmN;
	}


};

}

#endif /* OPENKAI_SRC_AUTOPILOT_COMMONAUTOPILOT_H_ */
