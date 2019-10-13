#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_posCtrl_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_posCtrl_H_

#include "../../../Base/common.h"
#include "../../../Control/PIDctrl.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_base.h"

namespace kai
{

enum POSCTRL_MODE
{
	pc_setV,
	pc_setP,
	pc_setVP,
};

class _APcopter_posCtrl: public _AutopilotBase
{
public:
	_APcopter_posCtrl();
	~_APcopter_posCtrl();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	void draw(void);
	int check(void);

	void setPos(vFloat4& vP);
	void setTargetPos(vFloat4& vTargetP);
	void setEnable(bool bEnable);

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

	bool	m_bEnable;
	bool	m_bFixYaw;
	POSCTRL_MODE m_mode;
	mavlink_set_position_target_local_ned_t m_spt;

	//roll, pitch, alt, yaw
	vFloat4 m_vP;
	vFloat4 m_vTargetP;
	vFloat3 m_vSpeed;	//constant speed in setVP mode

	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pAlt;
	float	m_vYaw;

};

}
#endif
