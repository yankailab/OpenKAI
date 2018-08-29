
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_thrust_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_thrust_H_

#include "../../../Base/common.h"
#include "../../../SLAM/_SlamBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

#define RC_IN(x) (abs(((int)x) - ((int)m_pwmMid))>(int)m_rcDeadband)

namespace kai
{

#define THRUST_OFF 2
#define THRUST_ON 1
#define THRUST_ALT 0

#define THRUST_FORWARD 0
#define THRUST_SET 1
#define THRUST_BACKWARD 2

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
	uint8_t pwmPos(uint16_t pwm);

	void resetAllPwm(void);

public:
	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pYaw;
	PIDctrl* m_pAlt;

	APcopter_base* m_pAP;
	_SlamBase* m_pSB;
	_Mavlink* m_pMavAP;
	_Mavlink* m_pMavGCS;

	vDouble3 m_pTarget;
	double m_targetMin;
	double m_targetMax;
	uint16_t m_pwmLow;
	uint16_t m_pwmMid;
	uint16_t m_pwmHigh;

	THRUST_CHANNEL m_tF;
	THRUST_CHANNEL m_tB;
	THRUST_CHANNEL m_tL;
	THRUST_CHANNEL m_tR;
	int	m_pwmAlt;

	uint64_t m_rcTimeOut;
	uint8_t m_switch;
	uint8_t m_action;
	double m_dMove;

};

}
#endif
