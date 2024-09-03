#ifndef OpenKAI_src_Utility_utilEvent_H_
#define OpenKAI_src_Utility_utilEvent_H_

namespace kai
{
	struct TIME_OUT
	{
		uint64_t m_tSet = 0;
		uint64_t m_tOut = 0;

		void setTout(uint64_t tOut)
		{
			m_tOut = tOut;
		}

		void startT(uint64_t tNow)
		{
			m_tSet = tNow;
		}

		void start(void)
		{
			m_tSet = getTbootUs();
		}

		bool bTout(uint64_t tNow)
		{
			IF_F(m_tSet + m_tOut > tNow);

			return true;
		}
	};

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
			IF_F(m_tInterval <= 0);
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

}
#endif
