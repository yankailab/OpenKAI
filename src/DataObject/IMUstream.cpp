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
		queue.push_back({tStamp, value});
		if (queue.size() > 1000)
		{
			queue.pop_front();
		}
		updateTstamp(tStamp);
	}

	uint64_t IMUstream::get(deque<IMU_DATA> &gyro, deque<IMU_DATA> &acc)
	{
		std::shared_lock lock(m_sMutex);
		gyro = m_dqGyro;
		acc = m_dqAcc;
		return getTstamp();
	}

	void IMUstream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
