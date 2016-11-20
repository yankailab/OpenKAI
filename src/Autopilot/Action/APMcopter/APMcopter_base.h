#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_BASE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_BASE_H_

#include "../../../Base/common.h"
#include "../../../Interface/_Mavlink.h"
#include "../ActionBase.h"



namespace kai
{

struct APMcopter_PID
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

struct APMcopter_CTRL
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

class APMcopter_base: public ActionBase
{
public:
	APMcopter_base();
	~APMcopter_base();

	bool init(Config* pConfig);
	bool link(void);
	void sendHeartbeat(void);

public:
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	APMcopter_PID m_pidRoll;
	APMcopter_PID m_pidPitch;
	APMcopter_PID m_pidYaw;
	APMcopter_PID m_pidAlt;

};

}

#endif

