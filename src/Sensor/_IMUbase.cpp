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
		m_nIMUdqMax = 1000;
		m_tIMUpairToleranceUs = 5000; // 5ms
	}

	_IMUbase::~_IMUbase()
	{
	}

	bool _IMUbase::init(const json &j)
	{
		IF_F(!_ModuleBase::init(j));

		jKv(j, "nIMUdqMax", m_nIMUdqMax);
		jKv(j, "tIMUpairToleranceUs", m_tIMUpairToleranceUs);

		return true;
	}

	bool _IMUbase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n;

		return true;
	}

	bool _IMUbase::start(void)
	{
		NULL_F(m_pT);

		return m_pT->start(getUpdate, this);
	}

	bool _IMUbase::check(void)
	{
		return _ModuleBase::check();
	}

	void _IMUbase::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
		}
	}

	void _IMUbase::addGyro(uint64_t tStamp, const vFloat3 &vG)
	{
		m_dqGyro.push_back({tStamp, vG});

		while (m_dqGyro.size() > m_nIMUdqMax)
			m_dqGyro.pop_front();
	}

	void _IMUbase::addAcc(uint64_t tStamp, const vFloat3 &vA)
	{
		m_dqAcc.push_back({tStamp, vA});

		while (m_dqAcc.size() > m_nIMUdqMax)
			m_dqAcc.pop_front();
	}

	uint64_t _IMUbase::getIMUpair(vFloat3 *pG, vFloat3 *pA)
	{
		NULL__(pG, 0);
		NULL__(pA, 0);

		while (!m_dqGyro.empty() && !m_dqAcc.empty())
		{
			uint64_t tG = m_dqGyro.front().m_t;
			uint64_t tA = m_dqAcc.front().m_t;

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

			*pG = m_dqGyro.front().m_v;
			*pA = m_dqAcc.front().m_v;

			m_dqGyro.pop_front();
			m_dqAcc.pop_front();

			return max(tG, tA);
		}

		return 0;
	}

	void _IMUbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		int nD = 3;
		if (!m_dqGyro.empty())
		{
			IMU_DATA G = m_dqGyro.back();
			pC->addMsg("vGyro = (" + lf2str(G.m_v.x, nD) + ", " + lf2str(G.m_v.y, nD) + ", " + lf2str(G.m_v.z, nD) + "), t=" + li2str(G.m_t));
		}

		if (!m_dqAcc.empty())
		{
			IMU_DATA A = m_dqAcc.back();
			pC->addMsg("vAcc  = (" + lf2str(A.m_v.x, nD) + ", " + lf2str(A.m_v.y, nD) + ", " + lf2str(A.m_v.z, nD) + "), t=" + li2str(A.m_t));
		}
	}
}
