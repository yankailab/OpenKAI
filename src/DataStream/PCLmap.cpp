#include "PCLmap.h"

namespace kai
{
	PCLmap::PCLmap()
		: m_snapshot(make_shared<Snapshot>())
	{
	}

	PCLmap::SnapshotPtr PCLmap::get(void) const
	{
		std::shared_lock lock(m_sMutex);
		return m_snapshot;
	}

	void PCLmap::set(vector<Submap> submaps, uint64_t session, uint64_t tStamp)
	{
		auto next = make_shared<Snapshot>();
		next->m_vSubmaps = std::move(submaps);
		next->m_session = session;
		next->m_tStamp = tStamp ? tStamp : getTns();
		SnapshotPtr previous;
		{
			std::unique_lock lock(m_sMutex);
			next->m_revision = m_snapshot->m_revision + 1;
			previous = std::move(m_snapshot);
			m_snapshot = std::move(next);
		}
	}
}
