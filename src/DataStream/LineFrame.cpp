/*
 * LineFrame.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "LineFrame.h"

namespace kai
{
	LineFrame::LineFrame()
		: m_snapshot(make_shared<Snapshot>())
	{
	}

	LineFrame::~LineFrame()
	{
	}

	LineFrame::SnapshotPtr LineFrame::get(void) const
	{
		std::shared_lock lock(m_sMutex);
		return m_snapshot;
	}

	void LineFrame::set(vector<GEOMETRY_LINE> lines, uint64_t tStamp)
	{
		auto next = make_shared<Snapshot>();
		next->m_vLines = std::move(lines);
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

	void LineFrame::console(void *pConsole)
	{
		NULL_(pConsole);
		DataStreamBase::console(pConsole);
	}

}
