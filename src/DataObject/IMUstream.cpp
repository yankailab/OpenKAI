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

	bool IMUstream::loadConfig(void)
	{
		IF_F(!this->DataObjBase::loadConfig());
		json &j = *m_pJ;

		jKv(j, "nBuf", m_nBuf);

		return true;
	}

	bool IMUstream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjBase::saveConfig(false));

		json &j = *m_pJ;
		j["nBuf"] = m_nBuf;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	void IMUstream::addGyro(const Vector3f &value, uint64_t tStamp)
	{
		if (!tStamp || !value.allFinite())
		{
			return;
		}

		std::unique_lock lock(m_sMutex);
		if (!m_dqGyro.empty() && tStamp <= m_dqGyro.back().m_t)
		{
			m_dqGyro.clear();
		}
		m_dqGyro.push_back({tStamp, value});

		if (m_dqGyro.size() > m_nBuf)
		{
			m_dqGyro.pop_front();
		}

		updateTstamp(tStamp);
	}

	void IMUstream::addAcc(const Vector3f &value, uint64_t tStamp)
	{
		if (!tStamp || !value.allFinite())
		{
			return;
		}

		std::unique_lock lock(m_sMutex);
		if (!m_dqAcc.empty() && tStamp <= m_dqAcc.back().m_t)
		{
			m_dqAcc.clear();
		}
		m_dqAcc.push_back({tStamp, value});

		if (m_dqAcc.size() > m_nBuf)
		{
			m_dqAcc.pop_front();
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
