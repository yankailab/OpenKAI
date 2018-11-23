#ifndef OpenKAI_src_Autopilot_Controller_APcopter_posCtrlTarget_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_posCtrlTarget_H_

#include "APcopter_posCtrlBase.h"

namespace kai
{

struct AP_POS_CTRL_TARGET
{
	PIDctrl* m_pPID;
	double m_v;

	void init(void)
	{
		m_pPID = NULL;
		m_v = 0.0;
	}

	bool update(double pos, double target)
	{
		NULL_F(m_pPID);
		m_v = m_pPID->update(pos, target);
		return true;
	}

	void clear(void)
	{
		m_v = 0.0;
		NULL_(m_pPID);
		m_pPID->reset();
	}
};

class APcopter_posCtrlTarget: public APcopter_posCtrlBase
{
public:
	APcopter_posCtrlTarget();
	~APcopter_posCtrlTarget();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

	void setTargetPos(vDouble4& vT);
	void setPos(vDouble4& vP);
	void clear(void);

	void releaseCtrl(void);

public:
	AP_POS_CTRL_TARGET m_ctrl[N_CTRL];
	mavlink_set_position_target_local_ned_t m_spt;

	double m_yawRate;
	bool	m_bSetV;

};

}
#endif
