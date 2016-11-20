#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMROVER_BASE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMROVER_BASE_H_

#include "../../../Base/common.h"
#include "../../../Interface/_Mavlink.h"
#include "../ActionBase.h"

namespace kai
{

struct APMrover_PID
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

struct APMrover_CTRL
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

class APMrover_base: public ActionBase
{
public:
	APMrover_base();
	~APMrover_base();

	bool init(Config* pConfig);
	bool link(void);
	void sendHeartbeat(void);

public:
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	APMrover_PID m_pidThrust;
	APMrover_PID m_pidSteer;

	APMrover_CTRL m_ctrlThrust;
	APMrover_CTRL m_ctrlSteer;

};

}

#endif

