#ifndef OpenKAI_src_Utility_RC_H_
#define OpenKAI_src_Utility_RC_H_

#include "../Base/common.h"

namespace kai
{

struct RC_CHANNEL
{
	uint8_t m_iChan = 0;

	uint16_t m_rawL = 1000;
	uint16_t m_rawM = 1500;
	uint16_t m_rawH = 2000;
	uint16_t m_raw = m_rawM;
	uint16_t m_rawDz = 1;
	float m_bH;
	float m_bL;
	float m_v = 0.5;	//normalized to 0.0 ~ 1.0

	vector<int> m_vDiv;
	int m_i = 0;

	void update(void)
	{
		m_bH = m_rawH - m_rawM;
		m_bH = 0.5/m_bH;

		m_bL = m_rawM - m_rawL;
		m_bL = 0.5/m_bL;
	}

	void set(uint16_t raw)
	{
		m_raw = constrain<uint16_t>(raw, m_rawL, m_rawH);

        int dRaw = (int)m_raw - (int)m_rawM;
        if(abs(dRaw) < m_rawDz)
        {
            m_v = 0.5;
        }
        else
        {
            if(dRaw > 0)
                m_v = (float)dRaw * m_bH + 0.5;
            else
                m_v = 0.5 + (float)dRaw * m_bL;
        }

		IF_(m_vDiv.empty());

		int i=0;
		for(int d : m_vDiv)
		{
			if(m_raw < d)break;
			i++;
		}
		m_i = i;
	}

	float v(void)
	{
		return m_v;
	}

	int raw(void)
	{
		return m_raw;
	}
	
	uint16_t i(void)
	{
		return m_i;
	}
};

}
#endif
