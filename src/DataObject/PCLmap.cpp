#include "PCLmap.h"

namespace kai
{
	PCLmap::PCLmap()
	{
	}

	void PCLmap::set(const vector<Submap> &src, uint64_t session, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);
		m_vSubmaps = src;
		m_session = session;
		updateTstamp(tStamp);
	}

	uint64_t PCLmap::get(vector<Submap> &dest, uint64_t &session)
	{
		std::shared_lock lock(m_sMutex);
		dest = m_vSubmaps;
		session = m_session;
		return getTstamp();
	}
}
