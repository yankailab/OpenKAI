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
#define N_RC_CHAN 4

struct AP_POS_CTRL_RC
{
	PIDctrl* m_pPID;
	int m_pwmL;
	int m_pwmM;
	int m_pwmH;
	bool m_bON;
	uint8_t m_iChan;
	uint16_t* m_pMavChanRaw;

	void init(void)
	{
		m_pPID = NULL;
		m_pwmL = 1000;
		m_pwmM = 1500;
		m_pwmH = 2000;
		m_bON = false;
		m_iChan = 0;
		m_pMavChanRaw = NULL;
	}

	void setRCChannel(mavlink_rc_channels_override_t* pRCO)
	{
		NULL_(pRCO);
		if(m_iChan == 1)
			m_pMavChanRaw = &pRCO->chan1_raw;
		else if(m_iChan == 2)
			m_pMavChanRaw = &pRCO->chan2_raw;
		else if(m_iChan == 3)
			m_pMavChanRaw = &pRCO->chan3_raw;
		else if(m_iChan == 4)
			m_pMavChanRaw = &pRCO->chan4_raw;
		else if(m_iChan == 5)
			m_pMavChanRaw = &pRCO->chan5_raw;
		else if(m_iChan == 6)
			m_pMavChanRaw = &pRCO->chan6_raw;
		else if(m_iChan == 7)
			m_pMavChanRaw = &pRCO->chan7_raw;
		else if(m_iChan == 8)
			m_pMavChanRaw = &pRCO->chan8_raw;
		else
			m_pMavChanRaw = NULL;
	}

	bool updatePWM(uint16_t pwm)
	{
		NULL_F(m_pMavChanRaw);
		IF_F(!m_bON);

		*m_pMavChanRaw = pwm;
		return true;
	}

	void bON(bool bON)
	{
		if(!m_bON && bON)
			m_pPID->reset();

		m_bON = bON;
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

	void setTargetPos(vDouble4& vT);
	void setPos(vDouble4& vP);
	void setCtrl(uint8_t iChan, bool bON);

	void bON(bool bON);
	void resetRC(void);
	void releaseRC(void);

public:
	APcopter_base* m_pAP;
	bool	m_bON;

	vDouble4 m_vTarget;
	vDouble4 m_vPos;

	AP_POS_CTRL_RC m_rc[N_RC_CHAN];
	mavlink_rc_channels_override_t m_rcO;

};

}
#endif
