/*
 * IMUstream.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "IMUstream.h"

namespace kai
{

	IMUstream::IMUstream()
	{
	}

	IMUstream::~IMUstream()
	{
	}

	void IMUstream::set(Type type, const Vector3f &value, uint64_t tStamp)
	{
		if (!tStamp || !value.allFinite())
		{
			return;
		}

		std::unique_lock lock(m_sMutex);
		auto &queue = type == Type::Gyro ? m_dqGyro : m_dqAcc;
		if (!queue.empty() && tStamp <= queue.back().m_t)
		{
			queue.clear();
		}
		queue.push_back({tStamp, value, ++m_sequence});
		if (queue.size() > 1000)
		{
			queue.pop_front();
		}
		m_tStamp = tStamp;
	}

	IMUstream::SnapshotPtr IMUstream::get(void) const
	{
		auto frame = make_shared<Snapshot>();
		{
			std::shared_lock lock(m_sMutex);
			frame->m_dqGyro = m_dqGyro;
			frame->m_dqAcc = m_dqAcc;
			frame->m_tStamp = m_tStamp;
			frame->m_revision = m_sequence;
		}
		return frame;
	}

	void IMUstream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataStreamBase::console(pConsole);
	}

}
