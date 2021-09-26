/*
 * Goto.cpp
 *
 *  Created on: March 16, 2019
 *      Author: yankai
 */

#include "Goto.h"

namespace kai
{

	Goto::Goto()
	{
		m_type = state_goto;
		m_hdg = 0;
		reset();
	}

	Goto::~Goto()
	{
	}

	bool Goto::init(void *pKiss)
	{
		IF_F(!this->State::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("hdg", &m_hdg);

		return true;
	}

	bool Goto::update(void)
	{
		this->State::update();

		IF_F(m_tTimeout <= 0);
		IF_F(m_tStamp < m_tStart + m_tTimeout);

		LOG_I("Timeout");
		return true;
	}

	void Goto::reset(void)
	{
		m_vPos.init();
		this->State::reset();
	}

	void Goto::setPos(vDouble3 &p)
	{
		m_vPos = p;
	}

	void Goto::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->State::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("Pos = (" + f2str(m_vPos.x, 7) + ", " + f2str(m_vPos.y, 7) + ", " + f2str(m_vPos.z, 7) + ")", 1);

		int tOut = (int)((double)(m_tTimeout - (m_tStamp - m_tStart)) * USEC_2_SEC);
		tOut = constrain(tOut, 0, INT_MAX);
		pC->addMsg("Timeout = " + i2str(tOut), 1);
#endif
	}

}
