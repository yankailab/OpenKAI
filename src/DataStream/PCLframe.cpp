/*
 * PCLframe.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "PCLframe.h"

namespace kai
{
	PCLframe::PCLframe()
		: m_snapshot(make_shared<Snapshot>())
	{
	}

	PCLframe::~PCLframe()
	{
	}

	PCLframe::SnapshotPtr PCLframe::get(void) const
	{
		std::shared_lock lock(m_sMutex);
		return m_snapshot;
	}

	void PCLframe::set(vector<GEOMETRY_POINT> points, uint64_t tStamp)
	{
		auto next = make_shared<Snapshot>();
		next->m_vPoints = std::move(points);
		next->m_tStamp = tStamp ? tStamp : getTns();
		SnapshotPtr previous;
		{
			std::unique_lock lock(m_sMutex);
			next->m_revision = m_snapshot->m_revision + 1;
			previous = std::move(m_snapshot);
			m_snapshot = std::move(next);
		}
		// A replaced frame is released outside the publication lock.
	}

	void PCLframe::console(void *pConsole)
	{
		NULL_(pConsole);
		DataStreamBase::console(pConsole);
	}

}
