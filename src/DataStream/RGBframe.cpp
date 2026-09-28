/*
 * RGBframe.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "RGBframe.h"

namespace kai
{
	RGBframe::RGBframe()
		: m_snapshot(make_shared<Snapshot>())
	{
	}

	RGBframe::~RGBframe()
	{
	}

	RGBframe::SnapshotPtr RGBframe::get(void) const
	{
		std::shared_lock lock(m_sMutex);
		return m_snapshot;
	}

	void RGBframe::set(const Mat &rgb, uint64_t tStamp)
	{
		auto next = make_shared<Snapshot>();
		next->m_mRGB = rgb.clone();
		next->m_tStamp = tStamp ? tStamp : getTns();
		SnapshotPtr previous;
		{
			std::unique_lock lock(m_sMutex);
			next->m_revision = m_snapshot->m_revision + 1;
			previous = std::move(m_snapshot);
			m_snapshot = std::move(next);
		}
		// A replaced image is released outside the publication lock.
	}

	void RGBframe::console(void *pConsole)
	{
		NULL_(pConsole);
		DataStreamBase::console(pConsole);
	}

}
