/*
 * _SLAMbase.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#include "_SLAMbase.h"
#include <stdexcept>

namespace kai
{
	_SLAMbase::_SLAMbase()
	{
		m_tConfidenceTimeoutNs = NSEC_SEC;
	}

	_SLAMbase::~_SLAMbase()
	{
		// Derived backends must also join before destroying their own resources.
		if (m_pT)
			m_pT->join();
	}

	bool _SLAMbase::loadConfig(void)
	{
		IF_F(!_NavBase::loadConfig());
		const json &j = *m_pJ;
		jKv(j, "bAutoStart", m_bAutoStart);
		return true;
	}

	bool _SLAMbase::saveConfig(bool bExport)
	{
		IF_F(!_NavBase::saveConfig(false));

		json &j = *m_pJ;
		j["bAutoStart"] = m_bAutoStart;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _SLAMbase::link(InstanceMgr *pM)
	{
		IF_F(!_NavBase::link(pM));
		const json &j = *m_pJ;
		string n;
		jKv(j, "PCLframeIn", n);
		m_pPCLin = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pPCLin, "Cannot find PCLframeIn: " + n);

		n.clear();
		jKv(j, "IMUstreamIn", n);
		m_pIMUin = n.empty() ? nullptr : dynamic_cast<IMUstream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pIMUin, "Cannot find IMUstreamIn: " + n);
		return true;
	}

	bool _SLAMbase::check(void)
	{
		return m_pPCLin && _NavBase::check();
	}

	bool _SLAMbase::start(void)
	{
		IF_F(!check());
		IF_F(m_bAutoStart && !startTracking());
		if (!m_pT->startThread(getUpdate, this))
		{
			stopTracking();
			return false;
		}
		return true;
	}

	void _SLAMbase::stop(void)
	{
		if (m_pT)
			m_pT->join();
		stopTracking();
	}

	bool _SLAMbase::startTracking(void)
	{
		auto lock = lockSLAM();
		IF__(m_bTracking, true);
		IF_F(!check());
		m_tStampLastFrame = 0;
		m_tPointInput = 0;
		m_dqGyro.clear();
		m_dqAcc.clear();
		m_bIMUbatch = false;
		m_tStampLastGyro = 0;
		m_tStampLatestGyro = 0;
		m_tStampLastAcc = 0;
		m_iGyro = 0;
		m_iAcc = 0;
		m_slamError.clear();
		m_tStampLastIMU = 0;
		setConfidence(0.0f);
		try
		{
			if (!startSLAM())
			{
				m_slamError = "Cannot initialize SLAM; check the backend log and sensor configuration";
				resetSLAM();
				return false;
			}
			m_bTracking = true;
			return true;
		}
		catch (const std::exception &e)
		{
			LOG_E(string("Cannot start SLAM: ") + e.what());
			m_slamError = e.what();
			resetSLAM();
			return false;
		}
	}

	bool _SLAMbase::bTracking(void)
	{
		return m_bTracking.load();
	}

	void _SLAMbase::stopTrackingLocked(void)
	{
		setConfidence(0.0f);
		IF_(!m_bTracking.exchange(false));
		try
		{
			stopSLAM();
		}
		catch (const std::exception &e)
		{
			LOG_E(string("Cannot finish SLAM: ") + e.what());
			m_slamError = e.what();
			resetSLAM();
		}
	}

	void _SLAMbase::stopTracking(void)
	{
		auto lock = lockSLAM();
		stopTrackingLocked();
	}

	void _SLAMbase::reset(void)
	{
		auto lock = lockSLAM();
		stopTrackingLocked();
		resetSLAM();
		m_tStampLastFrame = 0;
		m_tStampLastIMU = 0;
		m_tPointInput = 0;
		m_dqGyro.clear();
		m_dqAcc.clear();
		m_bIMUbatch = false;
		m_tStampLastGyro = 0;
		m_tStampLatestGyro = 0;
		m_tStampLastAcc = 0;
		m_iGyro = 0;
		m_iAcc = 0;
		m_slamError.clear();
		setPos(Vector3d::Zero());
		setOrientation(Quaterniond::Identity(), true);
	}

	bool _SLAMbase::readPointCloud(vector<GEOMETRY_POINT> &points, uint64_t &stamp)
	{
		if (!m_pPCLin || m_pPCLin->getTstamp() == m_tPointInput)
		{
			return false;
		}
		stamp = m_pPCLin->get(points);
		if (stamp == m_tPointInput)
		{
			return false;
		}
		m_tPointInput = stamp;
		
		// An empty frame is a clear, not a sensor measurement. Its host-clock
		// timestamp must not advance the estimator's capture clock.
		if (points.empty())
		{
			return false;
		}
		if (stamp < m_tStampLastFrame)
		{
			throw std::runtime_error("Point-cloud clock reset; restart SLAM tracking");
		}
		if (stamp == m_tStampLastFrame)
		{
			return false;
		}
		m_tStampLastFrame = stamp;
		return true;
	}

	bool _SLAMbase::readIMU(Vector3d &acc, Vector3d &gyro, uint64_t &stamp)
	{
		if (!m_pIMUin)
		{
			return false;
		}

		if (!m_bIMUbatch)
		{
			// Copy history once per batch. The two sensor channels can arrive
			// separately with equal capture times, so inspect both histories.
			m_pIMUin->get(m_dqGyro, m_dqAcc);
			if ((!m_dqGyro.empty() && m_dqGyro.back().m_t < m_tStampLatestGyro) ||
				(!m_dqAcc.empty() && m_dqAcc.back().m_t < m_tStampLastAcc))
			{
				throw std::runtime_error("IMU clock reset; restart SLAM tracking");
			}

			m_iGyro = 0;
			m_iAcc = 0;
			if (!m_dqAcc.empty()) m_tStampLastAcc = m_dqAcc.back().m_t;
			if (!m_dqGyro.empty()) m_tStampLatestGyro = m_dqGyro.back().m_t;
			while (m_iGyro < m_dqGyro.size() && m_dqGyro[m_iGyro].m_t <= m_tStampLastGyro)
			{
				++m_iGyro;
			}
			m_bIMUbatch = true;
		}

		// Accelerometers and gyroscopes can run at different rates (for example
		// 100/200 Hz). Preserve each gyro sample and interpolate acceleration
		// at its capture time, retaining both bracketing samples across batches.
		constexpr uint64_t maxAccGapNs = 100 * NSEC_MSEC;
		while (m_iGyro < m_dqGyro.size() && m_iAcc < m_dqAcc.size())
		{
			const auto &g = m_dqGyro[m_iGyro];
			if (g.m_t <= m_tStampLastGyro)
			{
				++m_iGyro;
				continue;
			}
			while (m_iAcc + 1 < m_dqAcc.size() && m_dqAcc[m_iAcc + 1].m_t <= g.m_t)
			{
				++m_iAcc;
			}
			const auto &before = m_dqAcc[m_iAcc];
			if (g.m_t < before.m_t)
			{
				// No earlier accelerometer sample remains in the bounded history.
				m_tStampLastGyro = g.m_t;
				++m_iGyro;
				continue;
			}
			if (g.m_t == before.m_t)
			{
				acc = before.m_v.cast<double>();
			}
			else
			{
				// Wait for the next acceleration callback; never extrapolate or
				// consume this gyro sample while its upper bracket is missing.
				if (m_iAcc + 1 == m_dqAcc.size()) break;
				const auto &after = m_dqAcc[m_iAcc + 1];
				const uint64_t interval = after.m_t - before.m_t;
				if (interval > maxAccGapNs)
				{
					m_tStampLastGyro = g.m_t;
					++m_iGyro;
					continue;
				}
				const double weight = double(g.m_t - before.m_t) / double(interval);
				acc = (1.0 - weight) * before.m_v.cast<double>() + weight * after.m_v.cast<double>();
			}

			m_tStampLastGyro = g.m_t;
			++m_iGyro;
			stamp = g.m_t;
			if (stamp < m_tStampLastIMU)
			{
				throw std::runtime_error("IMU clock reset; restart SLAM tracking");
			}
			if (stamp == m_tStampLastIMU) continue;
			m_tStampLastIMU = stamp;
			gyro = g.m_v.cast<double>();
			return true;
		}

		m_bIMUbatch = false;
		return false;
	}

	bool _SLAMbase::publishPose(const Isometry3d &pose, float confidence)
	{
		if (!pose.matrix().allFinite() || !pose.linear().isUnitary(1e-3) ||
			std::abs(pose.linear().determinant() - 1.0) > 1e-3)
		{
			setConfidence(0.0f);
			return false;
		}
		setPos(pose.translation());
		setOrientation(Quaterniond(pose.linear()), true);
		setConfidence(confidence);
		return true;
	}

	bool _SLAMbase::startSLAM(void) { return false; }
	void _SLAMbase::stopSLAM(void) {}
	void _SLAMbase::resetSLAM(void) {}
	void _SLAMbase::updateSLAM(void) {}

	std::unique_lock<std::mutex> _SLAMbase::lockSLAM(void)
	{
		++m_controlWaiters;
		std::unique_lock<std::mutex> lock(m_mtxSLAM);
		--m_controlWaiters;
		return lock;
	}

	void _SLAMbase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			IF_CONT(m_controlWaiters.load() > 0);
			std::lock_guard<std::mutex> lock(m_mtxSLAM);
			IF_CONT(!m_pT->bRun() || !m_bTracking);
			try
			{
				updateSLAM();
			}
			catch (const std::exception &e)
			{
				LOG_E(string("SLAM processing failed: ") + e.what());
				m_slamError = e.what();
				m_bTracking = false;
				setConfidence(0.0f);
				resetSLAM();
			}
		}
	}

	void _SLAMbase::console(void *pConsole)
	{
		NULL_(pConsole);
		_NavBase::console(pConsole);
		static_cast<_Console *>(pConsole)->addMsg(bTracking() ? "SLAM session active" : "SLAM session stopped");
	}
}
