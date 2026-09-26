/*
 * _IMUbase.cpp
 *
 *  Created on: March 5, 2026
 *      Author: yankai
 */

#include "_IMUbase.h"

namespace kai
{

	_IMUbase::_IMUbase()
	{
	}

	_IMUbase::~_IMUbase()
	{
		stop();
		DEL(m_pTstream);
	}

	bool _IMUbase::loadConfig(void)
	{
		IF_F(!_ModuleBase::loadConfig());
		json &j = *m_pJ;

		jKv(j, "nIMUdqMax", m_nIMUdqMax);
		jKv(j, "tIMUpairToleranceNs", m_tIMUpairToleranceNs);

		jKv(j, "bFusion", m_bFusion);
		IF_Le_F(m_nIMUdqMax < 1 || m_nIMUdqMax > 100000, "Invalid IMU queue size");

		if (!j.contains("threadStream"))
		{
			j["threadStream"] = json::object();
		}
		json &jStream = j["threadStream"];
		if (!jStream.is_object())
		{
			return false;
		}
		float fpsStream = 30.0f;
		jKv(jStream, "FPS", fpsStream);
		if (!std::isfinite(fpsStream) || fpsStream <= 0)
		{
			return false;
		}
		jStream["FPS"] = fpsStream;
		DEL(m_pTstream);
		m_pTstream = createThread(&jStream, "threadStream");
		NULL_F(m_pTstream);

		return true;
	}

	bool _IMUbase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		IF_F(!m_pTstream || !m_pTstream->link());

		return true;
	}

	bool _IMUbase::start(void)
	{
		NULL_F(m_pT);

		NULL_F(m_pTstream);
		IF_F(!m_pT->startThread(getUpdate, this));
		if (!m_pTstream->startThread(getUpdateStream, this))
		{
			m_pT->join();
			return false;
		}
		return true;
	}

	void _IMUbase::stop(void)
	{
		if (m_pT)
			m_pT->join();
		if (m_pTstream)
			m_pTstream->join();
		std::lock_guard<std::mutex> lock(m_mtxStream);
		m_pJb = nullptr;
	}

	bool _IMUbase::check(void)
	{
		return _ModuleBase::check();
	}

	void _IMUbase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			if (m_bFusion)
			{
				updateIMUfusion();
			}
		}
	}

	void _IMUbase::updateIMUfusion(void)
	{
		Vector3f gyro, acc;
		for (int n = 0; n < m_nIMUdqMax; ++n)
		{
			uint64_t t;
			{
				// A producer or preview reader may own the queue mutex. Retry on
				// the next pass rather than blocking this full-speed update loop.
				std::unique_lock<std::mutex> lock(m_mtxIMU, std::try_to_lock);
				if (!lock.owns_lock())
					break;
				t = pairFromQueues(m_dqFusionGyro, m_dqFusionAcc, &gyro, &acc);
			}
			if (!t)
				break;

			const uint64_t previous = m_tStampFusion;
			m_tStampFusion = t;
			m_bPreviewDirty = true;
			if (!previous || t <= previous || t - previous > NSEC_SEC / 2)
			{
				m_SF = SF();
				m_vOrt.setIdentity();
				m_vAngle.setZero();
				m_bOrientationValid = false;
				continue;
			}

			m_SF.MahonyUpdate(gyro.x(), gyro.y(), gyro.z(), acc.x(), acc.y(), acc.z(),
							  nsec2sec<float>(t - previous));
			const float *q = m_SF.getQuat(); // SF order: w, x, y, z.
			m_vOrt = Quaterniond(q[0], q[1], q[2], q[3]).normalized();
			// ZYX decomposition returns yaw/pitch/roll; publish roll/pitch/yaw.
			m_vAngle = m_vOrt.toRotationMatrix().canonicalEulerAngles(2, 1, 0).reverse();
			m_bOrientationValid = m_vOrt.coeffs().allFinite() && m_vAngle.allFinite();
		}

		if (m_bPreviewDirty)
		{
			std::unique_lock<std::mutex> lock(m_mtxPreview, std::try_to_lock);
			if (!lock.owns_lock())
				return;
			m_vOrtPreview = m_vOrt;
			m_vAnglePreview = m_vAngle;
			m_tFusionPreview = m_tStampFusion;
			m_bOrientationPreviewValid = m_bOrientationValid;
			m_bPreviewDirty = false;
		}
	}

	void _IMUbase::updateStream(void)
	{
		while (m_pTstream->bRun())
		{
			m_pTstream->autoFPS();
			streamIMUdata();
		}
	}

	void _IMUbase::streamIMUdata(void)
	{
		// This mutex orders stream commands and sends; fusion never uses it.
		std::lock_guard<std::mutex> streamLock(m_mtxStream);
		if (!m_pJb)
			return;

		IMU_DATA gyro, acc;
		Quaterniond orientation;
		Vector3d angles;
		uint64_t tFusion;
		bool orientationValid;
		{
			std::lock_guard<std::mutex> lock(m_mtxIMU);
			gyro = m_latestGyro;
			acc = m_latestAcc;
		}
		{
			std::lock_guard<std::mutex> lock(m_mtxPreview);
			orientation = m_vOrtPreview;
			angles = m_vAnglePreview;
			tFusion = m_tFusionPreview;
			orientationValid = m_bOrientationPreviewValid;
		}
		if (!gyro.m_t || !acc.m_t)
			return;
		if (gyro.m_t == m_tLastStreamG && acc.m_t == m_tLastStreamA && tFusion == m_tLastStreamFusion)
			return;
		m_tLastStreamG = gyro.m_t;
		m_tLastStreamA = acc.m_t;
		m_tLastStreamFusion = tFusion;

		// Allocate and send JSON only after releasing both snapshot mutexes.
		const auto &g = gyro.m_v;
		const auto &a = acc.m_v;
		const json j = {{"cmd", "imuData"}, {"module", getName()}, {"tGyro", gyro.m_t}, {"tAcc", acc.m_t}, {"tFusion", tFusion}, {"gyro", {g.x(), g.y(), g.z()}}, {"acc", {a.x(), a.y(), a.z()}}, {"quaternion", {orientation.w(), orientation.x(), orientation.y(), orientation.z()}}, {"rpy", {angles.x(), angles.y(), angles.z()}}, {"fusion", m_bFusion}, {"orientationValid", orientationValid}};
		if (!m_pJb->sendJson(j))
			m_pJb = nullptr;
	}

	void _IMUbase::addGyro(const Vector3f &vG, uint64_t tStamp)
	{
		if (!tStamp || !vG.allFinite())
			return;
		std::lock_guard<std::mutex> lock(m_mtxIMU);
		if (tStamp <= m_latestGyro.m_t)
		{
			m_dqGyro.clear();
			m_dqFusionGyro.clear();
		}
		m_latestGyro = {tStamp, vG};
		m_dqGyro.push_back(m_latestGyro);
		if (m_bFusion)
			m_dqFusionGyro.push_back(m_latestGyro);
		while (m_dqFusionGyro.size() > static_cast<size_t>(m_nIMUdqMax))
			m_dqFusionGyro.pop_front();

		while (m_nIMUdqMax >= 0 && m_dqGyro.size() > static_cast<size_t>(m_nIMUdqMax))
			m_dqGyro.pop_front();
	}

	void _IMUbase::addAcc(const Vector3f &vA, uint64_t tStamp)
	{
		if (!tStamp || !vA.allFinite())
			return;
		std::lock_guard<std::mutex> lock(m_mtxIMU);
		if (tStamp <= m_latestAcc.m_t)
		{
			m_dqAcc.clear();
			m_dqFusionAcc.clear();
		}
		m_latestAcc = {tStamp, vA};
		m_dqAcc.push_back(m_latestAcc);
		if (m_bFusion)
			m_dqFusionAcc.push_back(m_latestAcc);
		while (m_dqFusionAcc.size() > static_cast<size_t>(m_nIMUdqMax))
			m_dqFusionAcc.pop_front();

		while (m_nIMUdqMax >= 0 && m_dqAcc.size() > static_cast<size_t>(m_nIMUdqMax))
			m_dqAcc.pop_front();
	}

	uint64_t _IMUbase::getIMUpair(Vector3f *pG, Vector3f *pA)
	{
		NULL__(pG, 0);
		NULL__(pA, 0);
		std::lock_guard<std::mutex> lock(m_mtxIMU);
		return pairFromQueues(m_dqGyro, m_dqAcc, pG, pA);
	}

	uint64_t _IMUbase::pairFromQueues(deque<IMU_DATA> &gyro, deque<IMU_DATA> &acc, Vector3f *pG, Vector3f *pA)
	{
		while (!gyro.empty() && !acc.empty())
		{
			uint64_t tG = gyro.front().m_t;
			uint64_t tA = acc.front().m_t;

			if (tG < tA && tA - tG > m_tIMUpairToleranceNs)
			{
				gyro.pop_front();
				continue;
			}

			if (tA < tG && tG - tA > m_tIMUpairToleranceNs)
			{
				acc.pop_front();
				continue;
			}

			*pG = gyro.front().m_v;
			*pA = acc.front().m_v;

			gyro.pop_front();
			acc.pop_front();

			return max(tG, tA);
		}

		return 0;
	}

	void _IMUbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		IMU_DATA gyro, acc;
		Quaterniond orientation;
		Vector3d angles;
		{
			std::lock_guard<std::mutex> lock(m_mtxIMU);
			gyro = m_latestGyro;
			acc = m_latestAcc;
		}
		{
			std::lock_guard<std::mutex> lock(m_mtxPreview);
			orientation = m_vOrtPreview;
			angles = m_vAnglePreview;
		}

		_Console *pC = (_Console *)pConsole;
		const int nD = 3;
		if (gyro.m_t)
			pC->addMsg("vGyro = (" + lf2str(gyro.m_v.x(), nD) + ", " + lf2str(gyro.m_v.y(), nD) + ", " + lf2str(gyro.m_v.z(), nD) + "), t=" + li2str(gyro.m_t));
		if (acc.m_t)
			pC->addMsg("vAcc  = (" + lf2str(acc.m_v.x(), nD) + ", " + lf2str(acc.m_v.y(), nD) + ", " + lf2str(acc.m_v.z(), nD) + "), t=" + li2str(acc.m_t));
		if (m_bFusion)
		{
			pC->addMsg("Orientation wxyz: (" + lf2str(orientation.w(), nD) + ", " + lf2str(orientation.x(), nD) + ", " + lf2str(orientation.y(), nD) + ", " + lf2str(orientation.z(), nD) + ")");
			pC->addMsg("Roll/pitch/yaw (rad): (" + lf2str(angles.x(), nD) + ", " + lf2str(angles.y(), nD) + ", " + lf2str(angles.z(), nD) + ")");
		}
	}

	void _IMUbase::console(const json &j, void *pJSONbase)
	{
		auto *pJb = static_cast<_JSONbase *>(pJSONbase);
		if (!pJb || !j.is_object() || !j.contains("cmd") || !j["cmd"].is_string())
			return;
		const string cmd = j["cmd"].get<string>();
		if (cmd != "startStream" && cmd != "stopStream")
			return;
		std::lock_guard<std::mutex> lock(m_mtxStream);
		if (cmd == "startStream")
		{
			m_pJb = pJb;
			m_tLastStreamG = m_tLastStreamA = m_tLastStreamFusion = 0;
		}
		else
			m_pJb = nullptr;
		json reply = {{"cmd", cmd}, {"module", getName()}, {"bSuccess", true}, {"streaming", m_pJb != nullptr}};
		if (j.contains("requestId"))
			reply["requestId"] = j["requestId"];
		pJb->sendJson(reply);
	}

}
