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
		m_bTracking = false;
		m_nIMUdqMax = 1000;
		m_tIMUpairToleranceUs = 5000; // 5ms
	}

	_SLAMbase::~_SLAMbase()
	{
	}

	bool _SLAMbase::init(const json &j)
	{
		IF_F(!this->_NavBase::init(j));

		jKv(j, "nIMUdqMax", m_nIMUdqMax);
		jKv(j, "tIMUpairToleranceUs", m_tIMUpairToleranceUs);

		return true;
	}

	bool _SLAMbase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_NavBase::link(j, pM));

		return true;
	}

	bool _SLAMbase::check(void)
	{
		return this->_NavBase::check();
	}

	bool _SLAMbase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _SLAMbase::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
		}
	}

	void _SLAMbase::pushGyro(uint64_t t_us, const Eigen::Vector3d &g)
	{
		m_dqGyro.push_back({t_us, g});

		while (m_dqGyro.size() > m_nIMUdqMax)
			m_dqGyro.pop_front();
	}

	void _SLAMbase::pushAcc(uint64_t t_us, const Eigen::Vector3d &a)
	{
		m_dqAcc.push_back({t_us, a});

		while (m_dqAcc.size() > m_nIMUdqMax)
			m_dqAcc.pop_front();
	}

	uint64_t _SLAMbase::getIMUpair(Vector3d *pGyro, Vector3d *pAcc)
	{
		NULL__(pGyro, 0);
		NULL__(pAcc, 0);

		while (!m_dqGyro.empty() && !m_dqAcc.empty())
		{
			uint64_t tG = m_dqGyro.front().t_us;
			uint64_t tA = m_dqAcc.front().t_us;

			if (tG + m_tIMUpairToleranceUs < tA)
			{
				m_dqGyro.pop_front();
				continue;
			}

			if (tA + m_tIMUpairToleranceUs < tG)
			{
				m_dqAcc.pop_front();
				continue;
			}

			*pGyro = m_dqGyro.front().v;
			*pAcc = m_dqAcc.front().v;

			m_dqGyro.pop_front();
			m_dqAcc.pop_front();

			return max(tG, tA);
		}

		return false;
	}

	void _SLAMbase::pushPointCloud(const LidarScan& ls)
	{
		m_LS = ls;
	}

	bool _SLAMbase::bTracking(void)
	{
		return m_bTracking;
	}

	void _SLAMbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_NavBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		if (m_bTracking)
		{
			msg = "Tracking";
		}
		else
		{
			msg = "Tracking lost";
		}
		pC->addMsg(msg);

		int nD = 3;
		if (!m_dqGyro.empty())
		{
			TimedVec3 vGyro = m_dqGyro.back();
			pC->addMsg("vGyro = (" + lf2str(vGyro.v[0], nD) + ", " + lf2str(vGyro.v[1], nD) + ", " + lf2str(vGyro.v[2], nD) + "), t=" + li2str(vGyro.t_us));
		}

		if (!m_dqAcc.empty())
		{
			TimedVec3 vAcc = m_dqAcc.back();
			pC->addMsg("vAcc  = (" + lf2str(vAcc.v[0], nD) + ", " + lf2str(vAcc.v[1], nD) + ", " + lf2str(vAcc.v[2], nD) + "), t=" + li2str(vAcc.t_us));
		}
	}

	// void _SLAMbase::detect(void)
	// {
	// 	IF_(!check());

	// 	static const double usecBase = 1.0 / ((double)SEC_2_USEC);

	// 	Mat mGray;
	// 	m_pV->getFrameRGB()->m()->copyTo(mGray);
	// 	IF_(mGray.empty());

	// 	m_pose = m_pOS->TrackMonocular(mGray, t);
	// 	if (m_pose.empty())
	// 	{
	// 		m_bTracking = false;

	// 		// TODO: if lost too long reset the system

	// 		return;
	// 	}

	// 	// m_bTracking = true;
	// 	m_mRwc = m_pose.rowRange(0, 3).colRange(0, 3).t();
	// 	m_mTwc = -m_mRwc * m_pose.rowRange(0, 3).col(3);

	// 	m_vT.x = (double)m_mTwc.at<float>(0); // Right
	// 	m_vT.y = (double)m_mTwc.at<float>(1); // Down
	// 	m_vT.z = (double)m_mTwc.at<float>(2); // Front

	// 	Eigen::Matrix3f mRwc;
	// 	cv2eigen(m_mRwc, mRwc);
	// 	Eigen::Quaternionf qW;
	// 	qW = mRwc;

	// 	m_vQ.x = (double)qW.x();
	// 	m_vQ.y = (double)qW.y();
	// 	m_vQ.z = (double)qW.z();
	// 	m_vQ.w = (double)qW.w();

	// 	cout.precision(5);
	// 	cout << "vT: "
	// 		 << "  " << fixed << m_vT.x << "  " << fixed << m_vT.y << "  " << fixed << m_vT.z << "  "
	// 		 << "Q: "
	// 		 << "  " << fixed << m_vQ.x << "  " << fixed << m_vQ.y << "  " << fixed << m_vQ.z << "  " << fixed << m_vQ.w << endl;
	// }

}
