#ifndef OpenKAI_src_Base_data_H_
#define OpenKAI_src_Base_data_H_

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

}
#endif
