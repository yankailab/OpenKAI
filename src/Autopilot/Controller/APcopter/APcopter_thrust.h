
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_thrust_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_thrust_H_

#include "../../../Base/common.h"
#include "../../../SLAM/_SlamBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_thrust: public ActionBase
{
public:
	APcopter_thrust();
	~APcopter_thrust();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	void cmd(void);

public:
	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pYaw;
	PIDctrl* m_pAlt;

	APcopter_base* m_pAP;
	_SlamBase* m_pSB;
	_WebSocket* m_pCmd;

	vDouble3 m_pTarget;
	double m_targetMin;
	double m_targetMax;
	double m_dCollision;
	uint16_t m_pwmLow;
	uint16_t m_pwmMid;
	uint16_t m_pwmHigh;
	uint64_t m_rcTimeOut;

	mavlink_rc_channels_override_t m_rc;

};

}
#endif
