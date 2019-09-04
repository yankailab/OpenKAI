#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_posCtrl_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_posCtrl_H_

#include "../../../Base/common.h"
#include "../../../Control/PIDctrl.h"
#include "../../_ActionBase.h"
#include "_APcopter_base.h"

namespace kai
{

class _APcopter_posCtrl: public _ActionBase
{
public:
	_APcopter_posCtrl();
	~_APcopter_posCtrl();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

	void setPos(vFloat4& vP);
	void setTargetPos(vFloat4& vTargetP);
	void ctrlEnable(bool bON);

private:
	void clear(void);
	void releaseCtrl(void);
	void updateCtrl(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_posCtrl *) This)->update();
		return NULL;
	}

public:
	_APcopter_base* m_pAP;
	//roll, pitch, alt, yaw
	vFloat4 m_vP;
	vFloat4 m_vTargetP;

	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pAlt;
	double	m_vYaw;

	bool	m_bFixYaw;
	bool	m_bSetV;
	bool	m_bSetP;
	bool	m_bSetON;
	mavlink_set_position_target_local_ned_t m_spt;

};

}
#endif
