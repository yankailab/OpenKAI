/*
 * _SLAMbase.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#include "_SLAMbase.h"

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
		if (!_NavBase::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["bAutoStart"] = m_bAutoStart;

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _SLAMbase::link(void)
	{
		IF_F(!_NavBase::link());
		const json &j = *m_pJ;
		string n;
		jKv(j, "_PointCloud", n);
		m_pPCL = dynamic_cast<_PointCloud *>(static_cast<BASE *>(m_pM->findModule(n)));
		IF_Le_F(!m_pPCL, "Cannot find _PointCloud: " + n);

		n.clear();
		jKv(j, "_IMUbase", n);
		m_pIMU = n.empty() ? nullptr : dynamic_cast<_IMUbase *>(static_cast<BASE *>(m_pM->findModule(n)));
		IF_Le_F(!n.empty() && !m_pIMU, "Cannot find _IMUbase: " + n);
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
		if (m_bTracking)
			return true;
		IF_F(!check());
		m_tStampLastFrame = 0;
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
		m_tStampLastFrame = m_tStampLastIMU = 0;
		m_slamError.clear();
		setPos(Vector3d::Zero());
		setOrientation(Quaterniond::Identity(), true);
	}

	bool _SLAMbase::readPointCloud(vector<Vector3f> &points, uint64_t &stamp)
	{
		IF_F(!m_pPCL);
		const int count = m_pPCL->getLastFrameIfNew(&points, nullptr, stamp, m_tStampLastFrame);
		IF_F(count <= 0 || stamp <= m_tStampLastFrame);
		m_tStampLastFrame = stamp;
		return true;
	}

	bool _SLAMbase::readIMU(Vector3d &acc, Vector3d &gyro, uint64_t &stamp)
	{
		IF_F(!m_pIMU);
		Vector3f a, g;
		while ((stamp = m_pIMU->getIMUpair(&g, &a)) != 0)
		{
			IF_CONT(stamp <= m_tStampLastIMU || !a.allFinite() || !g.allFinite());
			m_tStampLastIMU = stamp;
			acc = a.cast<double>();
			gyro = g.cast<double>();
			return true;
		}
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
