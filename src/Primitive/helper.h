#ifndef OpenKAI_src_Base_helper_H_
#define OpenKAI_src_Base_helper_H_

namespace kai
{

struct INTERVAL_EVENT
{
	uint64_t m_tLastEvent;
	uint64_t m_tInterval;

	void init(uint64_t uInt)
	{
		m_tInterval = uInt;
		reset();
	}

	bool update(uint64_t tNow)
	{
		IF_F(tNow - m_tLastEvent < m_tInterval);

		m_tLastEvent = tNow;
		return true;
	}

	void reset(void)
	{
		m_tLastEvent = 0;
	}
};

struct STATE_CHANGE
{
	int m_state;
	bool m_bChanged;

	void init(int state)
	{
		m_state = state;
		m_bChanged = false;
	}

	void update(int state)
	{
		IF_(m_state == state);

		m_bChanged = true;
		m_state = state;
	}

	bool bActive(int state)
	{
		IF_F(m_state != state);
		IF_F(!m_bChanged);

		m_bChanged = false;
		return true;
	}
};

struct RC_STATE
{
	uint16_t m_pwm;
	vector<uint16_t> m_vP;
	int m_iState;

	void init(void)
	{
		m_pwm = UINT16_MAX;
		m_vP.clear();
		m_iState = -1;
	}

	bool addP(uint16_t p)
	{
		if(!m_vP.empty())
		{
			IF_F(p <= m_vP[m_vP.size()-1]);
		}

		m_vP.push_back(p);
		return true;
	}

	int update(uint16_t pwm)
	{
		m_pwm = pwm;

		if(m_vP.empty())
		{
			m_iState = -1;
			return m_iState;
		}

		int iState = 0;
		for(int p : m_vP)
		{
			if(m_pwm <= p)
				break;

			iState++;
		}

		m_iState = iState;
		return m_iState;
	}

	int state(void)
	{
		return m_iState;
	}
};

}
#endif
