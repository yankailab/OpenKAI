#ifndef OpenKAI_src_Autopilot_Rover_Rover_base_H_
#define OpenKAI_src_Autopilot_Rover_Rover_base_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../../Control/PIDctrl.h"
#include "../../_AutopilotBase.h"
#include "_Rover_CMD.h"

namespace kai
{

struct ROVER_DRIVE
{
	//output
	float m_nSpeed;
	float m_kDir;	//+/-1.0
	float m_sDir;	//+/-1.0
	uint8_t	 m_iPWM;
	uint16_t m_pwm;
	uint16_t m_pwmH;
	uint16_t m_pwmM;
	uint16_t m_pwmL;

	//feedback
	float m_nCurrent;
	float m_nRot;

	void init(void)
	{
		m_nSpeed = 0.0;
		m_kDir = 1.0;
		m_sDir = 1.0;
		m_iPWM = 0;
		m_pwmM = 1500;
		m_pwmH = 2000;
		m_pwmL = 1000;
		m_pwm = m_pwmM;

		m_nCurrent = -1.0;
		m_nRot = 0.0;
	}

	uint16_t updatePWM(float nSpeed, float dSpeed)
	{
		float k = nSpeed * m_kDir + dSpeed * m_sDir;
		k = constrain<float>(k, -1.0, 1.0);

		if(k >= 0.0)
			m_pwm = m_pwmM + ((float)(m_pwmH - m_pwmM)) * abs(k);
		else
			m_pwm = m_pwmM - ((float)(m_pwmM - m_pwmL)) * abs(k);

		return m_pwm;
	}

};

struct ROVER_CTRL
{
	float m_hdg;
	float m_targetHdg;
	float m_nSpeed;		//-1.0 to 1.0
	float m_nTargetSpeed;

	vector<ROVER_DRIVE> m_vDrive;

	void init(void)
	{
		m_hdg = -1.0;
		m_targetHdg = -1.0;
		m_nSpeed = 0.0;
		m_nTargetSpeed = 0.0;
	}
};

class _Rover_base: public _AutopilotBase
{
public:
	_Rover_base();
	~_Rover_base();

	bool init(void* pKiss);
	int	 check(void);
	void update(void);
	void draw(void);

	void setSpeed(float nSpeed);
	void setPinout(uint8_t pin, uint8_t status);

public:
	_Rover_CMD* m_pCMD;
	_Mavlink* m_pMavlink;
	PIDctrl* m_pPIDhdg;

	ROVER_CTRL m_ctrl;

};

}
#endif
