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
		m_pPCL = dynamic_cast<PCLframe *>(static_cast<DataStreamBase *>(pM->findDataStream(n)));
		IF_Le_F(!m_pPCL, "Cannot find PCLframeIn: " + n);

		n.clear();
		jKv(j, "IMUstream", n);
		m_pIMU = n.empty() ? nullptr : dynamic_cast<IMUstream *>(static_cast<DataStreamBase *>(pM->findDataStream(n)));
		IF_Le_F(!n.empty() && !m_pIMU, "Cannot find IMUstream: " + n);
		return true;
	}

	bool _SLAMbase::check(void)
	{
		return m_pPCL && _NavBase::check();
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
		m_pointRevision = 0;
		m_imuSnapshot.reset();
		m_gyroSequence = 0;
		m_accSequence = 0;
		m_tStampLastGyro = 0;
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
		m_pointRevision = 0;
		m_imuSnapshot.reset();
		m_gyroSequence = 0;
		m_accSequence = 0;
		m_tStampLastGyro = 0;
		m_tStampLastAcc = 0;
		m_iGyro = 0;
		m_iAcc = 0;
		m_slamError.clear();
		setPos(Vector3d::Zero());
		setOrientation(Quaterniond::Identity(), true);
	}

	PCLframe::SnapshotPtr _SLAMbase::readPointCloud(void)
	{
		if (!m_pPCL)
		{
			return nullptr;
		}
		auto frame = m_pPCL->get();
		if (frame->m_revision == m_pointRevision)
		{
			return nullptr;
		}
		m_pointRevision = frame->m_revision;
		// Clearing a stream is not a sensor measurement and must not advance
		// the estimator clock (the clear may use the host clock).
		if (frame->m_vPoints.empty())
		{
			return nullptr;
		}
		if (frame->m_tStamp < m_tStampLastFrame)
		{
			throw std::runtime_error("Point-cloud clock reset; restart SLAM tracking");
		}
		if (frame->m_tStamp == m_tStampLastFrame)
		{
			return nullptr;
		}
		m_tStampLastFrame = frame->m_tStamp;
		return frame;
	}

	bool _SLAMbase::readIMU(Vector3d &acc, Vector3d &gyro, uint64_t &stamp)
	{
		if (!m_pIMU)
		{
			return false;
		}
		if (!m_imuSnapshot)
		{
			// One history snapshot per processing batch, shared across this loop's
			// calls. Independent estimators retain their own sequence positions.
			m_imuSnapshot = m_pIMU->get();
			m_iGyro = 0;
			m_iAcc = 0;
			while (m_iGyro < m_imuSnapshot->m_dqGyro.size() &&
				m_imuSnapshot->m_dqGyro[m_iGyro].m_sequence <= m_gyroSequence)
			{
				++m_iGyro;
			}
			while (m_iAcc < m_imuSnapshot->m_dqAcc.size() &&
				m_imuSnapshot->m_dqAcc[m_iAcc].m_sequence <= m_accSequence)
			{
				++m_iAcc;
			}
		}

		constexpr uint64_t toleranceNs = 5 * NSEC_MSEC;
		while (m_iGyro < m_imuSnapshot->m_dqGyro.size() && m_iAcc < m_imuSnapshot->m_dqAcc.size())
		{
			const auto &g = m_imuSnapshot->m_dqGyro[m_iGyro];
			const auto &a = m_imuSnapshot->m_dqAcc[m_iAcc];
			// Detect independently resetting sensor clocks before discarding an
			// unmatched sample. The paired maximum hides a reset in one channel.
			if (g.m_t < m_tStampLastGyro || a.m_t < m_tStampLastAcc)
			{
				throw std::runtime_error("IMU clock reset; restart SLAM tracking");
			}
			if (g.m_t < a.m_t && a.m_t - g.m_t > toleranceNs)
			{
				m_gyroSequence = g.m_sequence;
				m_tStampLastGyro = g.m_t;
				++m_iGyro;
				continue;
			}
			if (a.m_t < g.m_t && g.m_t - a.m_t > toleranceNs)
			{
				m_accSequence = a.m_sequence;
				m_tStampLastAcc = a.m_t;
				++m_iAcc;
				continue;
			}

			m_gyroSequence = g.m_sequence;
			m_accSequence = a.m_sequence;
			m_tStampLastGyro = g.m_t;
			m_tStampLastAcc = a.m_t;
			++m_iGyro;
			++m_iAcc;
			stamp = std::max(g.m_t, a.m_t);
			if (stamp < m_tStampLastIMU)
			{
				throw std::runtime_error("IMU clock reset; restart SLAM tracking");
			}
			if (stamp == m_tStampLastIMU)
			{
				continue;
			}
			m_tStampLastIMU = stamp;
			acc = a.m_v.cast<double>();
			gyro = g.m_v.cast<double>();
			return true;
		}
		m_imuSnapshot.reset();
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
