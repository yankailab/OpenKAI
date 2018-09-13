#ifndef OpenKAI_src_Autopilot_Controller_APcopter_posCtrlRC_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_posCtrlRC_H_

#include "../../../Base/common.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

#define RC_CHAN_ROLL 1
#define RC_CHAN_PITCH 0
#define RC_CHAN_YAW 3
#define RC_CHAN_ALT 2

struct AP_POS_CTRL_RC
{
	PIDctrl* m_pPID;
	int m_pwmL;
	int m_pwmM;
	int m_pwmH;

	void init(void)
	{
		m_pPID = NULL;
		m_pwmL = 1000;
		m_pwmM = 1500;
		m_pwmH = 2000;
	}
};

class APcopter_posCtrlRC: public ActionBase
{
public:
	APcopter_posCtrlRC();
	~APcopter_posCtrlRC();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

	void setTargetPos(vDouble4& vT, uint8_t fSet);
	void setPos(vDouble4& vP);

	void bON(bool bON);
	void resetRC(void);
	void releaseRC(void);

public:
	APcopter_base* m_pAP;
	uint8_t m_fCtrl;
	bool	m_bON;

	vDouble4 m_vTarget;
	vDouble4 m_vPos;

	AP_POS_CTRL_RC m_rcRoll;
	AP_POS_CTRL_RC m_rcPitch;
	AP_POS_CTRL_RC m_rcYaw;
	AP_POS_CTRL_RC m_rcAlt;
	mavlink_rc_channels_override_t m_rc;

};

}
#endif
