#ifndef OpenKAI_src_Utility_RC_H_
#define OpenKAI_src_Utility_RC_H_

#include "../Base/common.h"

namespace kai
{

struct RC_CHANNEL
{
	uint8_t m_iChan;

	uint16_t m_pwmL;
	uint16_t m_pwmM;
	uint16_t m_pwmH;
	uint16_t m_pwm;
	float m_bH;
	float m_bL;
	float m_v;	//normalized to 0.0 ~ 1.0

	vector<int> m_vDiv;
	uint16_t m_i;

	void init(void)
	{
		m_iChan = 0;
		m_pwmL = 1000;
		m_pwmM = 1500;
		m_pwmH = 2000;
		m_pwm = m_pwmM;
		m_v = 0.5;
		m_i = 0;
	}

	void update(void)
	{
		m_bH = m_pwmH - m_pwmM;
		m_bH = 0.5/m_bH;

		m_bL = m_pwmM - m_pwmL;
		m_bL = 0.5/m_bL;
	}

	void pwm(uint16_t pwm)
	{
		pwm = constrain(pwm, m_pwmL, m_pwmH);

		if(pwm >= m_pwmM)
			m_v = (pwm - m_pwmM) * m_bH + 0.5;
		else
			m_v = 0.5 - (m_pwmM - pwm) * m_bL;

		IF_(m_vDiv.empty());

		int i=0;
		for(int d : m_vDiv)
		{
			if(pwm < d)break;
			i++;
		}
		m_i = i;
	}

	float v(void)
	{
		return m_v;
	}

	uint16_t i(void)
	{
		return m_i;
	}
};

}
#endif
