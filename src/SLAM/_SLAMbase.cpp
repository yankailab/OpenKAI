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
		m_tConfidenceTimeoutUs = 1000000;
	}

	_SLAMbase::~_SLAMbase()
	{
		// Derived backends must also join before destroying their own resources.
		if (m_pT)
			m_pT->join();
	}

	bool _SLAMbase::init(const json &j)
	{
		IF_F(!_NavBase::init(j));
		jKv(j, "bAutoStart", m_bAutoStart);
		return true;
	}

	bool _SLAMbase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!_NavBase::link(j, pM));
		string n;
		jKv(j, "_PointCloud", n);
		m_pPCL = dynamic_cast<_PointCloud *>(static_cast<BASE *>(pM->findModule(n)));
		IF_Le_F(!m_pPCL, "Cannot find _PointCloud: " + n);

		n.clear();
		jKv(j, "_IMUbase", n);
		m_pIMU = n.empty() ? nullptr : dynamic_cast<_IMUbase *>(static_cast<BASE *>(pM->findModule(n)));
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
		std::lock_guard<std::mutex> lock(m_mtxSLAM);
		if (m_bTracking)
			return true;
		IF_F(!check());
		m_tStampLastFrame = 0;
		m_tStampLastIMU = 0;
		setConfidence(0.0f);
		try
		{
			if (!startSLAM())
			{
				resetSLAM();
				return false;
			}
			m_bTracking = true;
			return true;
		}
		catch (const std::exception &e)
		{
			LOG_E(string("Cannot start SLAM: ") + e.what());
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
			resetSLAM();
		}
	}

	void _SLAMbase::stopTracking(void)
	{
		std::lock_guard<std::mutex> lock(m_mtxSLAM);
		stopTrackingLocked();
	}

	void _SLAMbase::reset(void)
	{
		std::lock_guard<std::mutex> lock(m_mtxSLAM);
		stopTrackingLocked();
		resetSLAM();
		m_tStampLastFrame = m_tStampLastIMU = 0;
		setPos(Vector3d::Zero());
		setOrientation(Quaterniond::Identity(), true);
	}

	bool _SLAMbase::readPointCloud(vector<Vector3f> &points, uint64_t &stamp)
	{
		IF_F(!m_pPCL);
		const int count = m_pPCL->getLastFrame(&points, nullptr, stamp);
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

	void _SLAMbase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			std::lock_guard<std::mutex> lock(m_mtxSLAM);
			IF_CONT(!m_pT->bRun() || !m_bTracking);
			try
			{
				updateSLAM();
			}
			catch (const std::exception &e)
			{
				LOG_E(string("SLAM processing failed: ") + e.what());
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
