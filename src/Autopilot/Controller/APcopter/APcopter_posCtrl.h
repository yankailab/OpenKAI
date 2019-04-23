#ifndef OpenKAI_src_Autopilot_Controller_APcopter_posCtrl_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_posCtrl_H_

#include "../../../Base/common.h"
#include "../../../Control/PIDctrl.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

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

	void setPos(vDouble4& vMyPos, vDouble4& vTargetPos);
	void clear(void);
	void releaseCtrl(void);
	void setON(bool bON);

public:
	APcopter_base* m_pAP;
	//roll, pitch, alt, yaw
	vDouble4 m_vTargetPos;
	vDouble4 m_vMyPos;

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
