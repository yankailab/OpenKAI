#ifndef OpenKAI_src_Autopilot_Rover__Rover_drive_H_
#define OpenKAI_src_Autopilot_Rover__Rover_drive_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "_Rover_base.h"

namespace kai
{

struct ROVER_DRIVE
{
	//output
	float m_nSpeed;
	float m_kDir;	//+/-1.0
	float m_sDir;	//+/-1.0
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

class _Rover_drive: public _AutopilotBase
{
public:
	_Rover_drive();
	~_Rover_drive();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updatePWM(void);

public:
	_Rover_CMD* m_pCMD;
	PIDctrl* m_pPID;

	ROVER_CTRL m_ctrl;
	vector<ROVER_DRIVE> m_vDrive;
	uint16_t* m_pPwmOut;

};

}
#endif
