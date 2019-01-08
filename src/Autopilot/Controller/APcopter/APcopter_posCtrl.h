#ifndef OpenKAI_src_Autopilot_Controller_APcopter_posCtrl_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_posCtrl_H_

#include "../../../Base/common.h"
#include "../../../Control/PIDctrl.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

#define C_PITCH 0
#define C_ROLL 1
#define C_ALT 2
#define C_YAW 3
#define N_CTRL 4

struct AP_POS_CTRL_TARGET
{
	PIDctrl* m_pPID;
	double m_v;

	void init(void)
	{
		m_pPID = NULL;
		m_v = 0.0;
	}

	bool update(double v, double vTarget)
	{
		NULL_F(m_pPID);
		m_v = m_pPID->update(v, vTarget);
		return true;
	}

	void clear(void)
	{
		m_v = 0.0;
		NULL_(m_pPID);
		m_pPID->reset();
	}
};

class APcopter_posCtrl: public ActionBase
{
public:
	APcopter_posCtrl();
	~APcopter_posCtrl();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

	void setTargetPos(vDouble4& vP);
	void setPos(vDouble4& vP);
	void clear(void);

	void releaseCtrl(void);

public:
	APcopter_base* m_pAP;
	vDouble4 m_vTarget;
	vDouble4 m_vPos;

	AP_POS_CTRL_TARGET m_ctrl[N_CTRL];
	mavlink_set_position_target_local_ned_t m_spt;

	double	m_vYaw;
	bool	m_bSetV;
	bool	m_bSetP;

};

}
#endif
