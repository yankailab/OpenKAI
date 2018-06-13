
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_thrust_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_thrust_H_

#include "../../../Base/common.h"
#include "../../../SLAM/_SlamBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

#define RC_IN(x) (abs(((int)x) - ((int)m_pwmMid))>(int)m_rcDeadband)

namespace kai
{

struct THRUST_CHANNEL
{
	uint8_t m_iChan;
	int	m_pwm;
	int m_sign;
};

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

	void resetAllPwm(void);

public:
	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pYaw;
	PIDctrl* m_pAlt;

	APcopter_base* m_pAP;
	_SlamBase* m_pSB;
	_WebSocket* m_pCmd;
	_Mavlink* m_pMavAP;
	_Mavlink* m_pMavGCS;

	vDouble3 m_pTarget;
	double m_targetMin;
	double m_targetMax;
	double m_dCollision;
	uint16_t m_pwmLow;
	uint16_t m_pwmMid;
	uint16_t m_pwmHigh;
	uint64_t m_rcTimeOut;
	uint16_t m_rcDeadband;
	uint32_t m_enableAPmode;

	THRUST_CHANNEL m_tF;
	THRUST_CHANNEL m_tB;
	THRUST_CHANNEL m_tL;
	THRUST_CHANNEL m_tR;
	int	m_pwmAlt;

};

}
#endif
