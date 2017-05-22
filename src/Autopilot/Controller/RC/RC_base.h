
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_RC_BASE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_RC_BASE_H_

#include "../../../Base/common.h"
#include "../../../Script/Kiss.h"
#include "../../ActionBase.h"

#define NUM_RC_CHANNEL 8

namespace kai
{

struct RC_PID
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

struct RC_CTRL
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

class RC_base: public ActionBase
{
public:
	RC_base();
	~RC_base();

	bool init(void* pKiss);

public:
	RC_PID m_pidRoll;
	RC_PID m_pidPitch;
	RC_PID m_pidYaw;
	RC_PID m_pidAlt;

	RC_CHANNEL m_rcRoll;
	RC_CHANNEL m_rcPitch;
	RC_CHANNEL m_rcYaw;
	RC_CHANNEL m_rcAlt;

};

}

#endif

