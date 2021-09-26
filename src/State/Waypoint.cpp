/*
 * Waypoint.cpp
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#include "Waypoint.h"

namespace kai
{

	Waypoint::Waypoint()
	{
		m_iWP = 0;
		m_dWP = 1;
		m_loop = wp_loop_none,
		m_vPos.init(-1.0);
		m_vErr.init(-1.0);

		m_type = state_wp;
	}

	Waypoint::~Waypoint()
	{
	}

	bool Waypoint::init(void *pKiss)
	{
		IF_F(!this->State::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("loop", (int *)&m_loop);

		int i = 0;
		while (1)
		{
			Kiss *pP = pK->child(i++);
			if (pP->empty())
				break;

			STATE_WAYPOINT p;
			p.init();
			pP->v("vP", &p.m_vP);
			pP->v("vV", &p.m_vV);
			pP->v("vErr", &p.m_vErr);

			m_vWP.push_back(p);
		}

		return true;
	}

	int Waypoint::check(void)
	{
		IF__(m_iWP < 0, -1);
		IF__(m_iWP >= m_vWP.size(), -1);
		IF__(m_vWP.empty(), -1);

		return 0;
	}

	void Waypoint::setPos(vDouble4 &vPos)
	{
		m_vPos = vPos;
	}

	STATE_WAYPOINT *Waypoint::getWaypoint(void)
	{
		IF__(check() < 0, NULL);

		return &m_vWP[m_iWP];
	}

	int Waypoint::getClosestWPidx(void)
	{
		double minD = DBL_MAX;
		int idx = -1;

		for (int i = 0; i < m_vWP.size(); i++)
		{
			STATE_WAYPOINT *pWP = &m_vWP[i];
			double d = abs(pWP->m_vP.x - m_vPos.x) + abs(pWP->m_vP.y - m_vPos.y);
			IF_CONT(d > minD);

			minD = d;
			idx = i;
		}

		return idx;
	}

	bool Waypoint::update(void)
	{
		IF_F(check() < 0);

		STATE_WAYPOINT *pWP = &m_vWP[m_iWP];
		IF_F(!pWP->update(m_vPos, &m_vErr));
		LOG_I("WP: " + i2str(m_iWP) + " complete");

		m_iWP += m_dWP;

		if (m_iWP < 0)
		{
			IF_T(m_loop == wp_loop_none);

			if (m_loop == wp_loop_repeat)
			{
				m_iWP = m_vWP.size() - 1;
			}
			else if (m_loop == wp_loop_shuttle)
			{
				m_iWP = 0;
				m_dWP = 1;
			}
		}
		else if (m_iWP >= m_vWP.size())
		{
			IF_T(m_loop == wp_loop_none);

			if (m_loop == wp_loop_repeat)
			{
				m_iWP = 0;
			}
			else if (m_loop == wp_loop_shuttle)
			{
				m_iWP = m_vWP.size() - 1;
				m_dWP = -1;
			}
		}

		return false;
	}

	void Waypoint::reset(void)
	{
		this->State::reset();

		m_iWP = 0;
	}

	void Waypoint::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->State::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		pC->addMsg("nWP=" + i2str(m_vWP.size()) + ", iWP=" + i2str(m_iWP), 1);

		IF_(check() < 0);

		STATE_WAYPOINT *pWP = &m_vWP[m_iWP];

		pC->addMsg("WP = (" + f2str(pWP->m_vP.x, 7) + ", " + f2str(pWP->m_vP.y, 7) + ", " + f2str(pWP->m_vP.x, 7) + ", " + f2str(pWP->m_vP.w, 7) + ")", 1);

		pC->addMsg("Pos = (" + f2str(m_vPos.x, 7) + ", " + f2str(m_vPos.y, 7) + ", " + f2str(m_vPos.z, 7) + ", " + f2str(m_vPos.w, 7) + ")", 1);

		pC->addMsg("Err = (" + f2str(m_vErr.x, 7) + ", " + f2str(m_vErr.y, 7) + ", " + f2str(m_vErr.z, 7) + ", " + f2str(m_vErr.w, 7) + ")", 1);
#endif
	}

}
