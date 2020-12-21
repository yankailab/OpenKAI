#ifndef OpenKAI_src_Utility_RC_H_
#define OpenKAI_src_Utility_RC_H_

#include "../Base/common.h"

namespace kai
{

struct RC_CHANNEL
{
	uint8_t m_iChan;

	int m_pwmL;
	int m_pwmM;
	int m_pwmH;
	int m_pwm;
	int m_pwmDz;
	float m_bH;
	float m_bL;
	float m_v;	//normalized to 0.0 ~ 1.0

	vector<int> m_vDiv;
	int m_i;

	void init(void)
	{
		m_iChan = 0;
		m_pwmL = 1000;
		m_pwmM = 1500;
		m_pwmH = 2000;
		m_pwm = m_pwmM;
        m_pwmDz = 1;

        m_v = 0.5;
		m_i = 0;
	}

	void setup(void)
	{
		m_bH = m_pwmH - m_pwmM;
		m_bH = 0.5/m_bH;

		m_bL = m_pwmM - m_pwmL;
		m_bL = 0.5/m_bL;
	}

	void pwm(int pwm)
	{
		m_pwm = constrain(pwm, m_pwmL, m_pwmH);

        int dPwm = m_pwm - m_pwmM;
        if(abs(dPwm) < m_pwmDz)
        {
            m_v = 0.5;
        }
        else
        {
            if(dPwm > 0)
                m_v = (float)dPwm * m_bH + 0.5;
            else
                m_v = 0.5 + (float)dPwm * m_bL;
        }

		IF_(m_vDiv.empty());

		int i=0;
		for(int d : m_vDiv)
		{
			if(m_pwm < d)break;
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
