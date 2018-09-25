#ifndef OpenKAI_src_Autopilot_Controller_APcopter_posCtrlRC_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_posCtrlRC_H_

#include "APcopter_posCtrlBase.h"

namespace kai
{

struct AP_POS_CTRL_RC
{
	PIDctrl* m_pPID;
	int m_pwmL;
	int m_pwmM;
	int m_pwmH;
	uint8_t m_iChan;
	uint16_t* m_pMavChanRaw;

	void init(void)
	{
		m_pPID = NULL;
		m_pwmL = 1000;
		m_pwmM = 1500;
		m_pwmH = 2000;
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

	bool update(double pos, double target)
	{
		NULL_F(m_pPID);
		NULL_F(m_pMavChanRaw);

		*m_pMavChanRaw = (uint16_t)(m_pwmM + (int)m_pPID->update(pos, target));
		return true;
	}

	void clear(void)
	{
		if(m_pPID)
			m_pPID->reset();

		if(m_pMavChanRaw)
			*m_pMavChanRaw = 0;
	}
};

class APcopter_posCtrlRC: public APcopter_posCtrlBase
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
	void clear(void);

	void resetRC(void);
	void releaseRC(void);

public:
	AP_POS_CTRL_RC m_rc[N_CTRL];
	mavlink_rc_channels_override_t m_rcO;

};

}
#endif
