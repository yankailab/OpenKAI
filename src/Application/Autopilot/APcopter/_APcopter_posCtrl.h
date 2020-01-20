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

	virtual bool init(void* pKiss);
	virtual void draw(void);
	virtual int check(void);

	void clear(void);
	void releaseCtrl(void);
	void updateCtrl(void);

public:
	_APcopter_base* m_pAP;

	bool	m_bFixYaw;
	POSCTRL_MODE m_mode;
	mavlink_set_position_target_local_ned_t m_spt;

	//roll, pitch, alt, yaw
	vFloat4 m_vP;		//variable screen coordinate of the object being followed in roll, pitch, alt, yaw order
	vFloat4 m_vTargetP;	//constant screen coordinate where the followed object should get to
	vFloat3 m_vSpeed;	//constant speed in setVP mode

	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pAlt;
	float	m_vYaw;

};

}
#endif
