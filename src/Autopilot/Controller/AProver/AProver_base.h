#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APROVER_BASE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APROVER_BASE_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../ActionBase.h"

namespace kai
{

struct AProver_PID
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

struct AProver_CTRL
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

class AProver_base: public ActionBase
{
public:
	AProver_base();
	~AProver_base();

	bool init(void* pKiss);
	bool link(void);
	void sendHeartbeat(void);
	void sendSteerThrust(void);

public:
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	AProver_PID m_pidThrust;
	AProver_PID m_pidSteer;

	AProver_CTRL m_ctrlThrust;
	AProver_CTRL m_ctrlSteer;

	double	m_thrust;
	double	m_steer;

};

}

#endif

