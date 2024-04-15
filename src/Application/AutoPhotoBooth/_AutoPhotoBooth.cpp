/*
 * _AutoPhotoBooth.cpp
 *
 *  Created on: Dec 24, 2022
 *      Author: yankai
 */

#include "../RobotArm/_AutoPhotoBooth.h"

namespace kai
{

	_AutoPhotoBooth::_AutoPhotoBooth()
	{
		m_bON = false;
		m_tResume = 0;
		m_bComplete = false;
	}

	_AutoPhotoBooth::~_AutoPhotoBooth()
	{
	}

	bool _AutoPhotoBooth::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pAction = pK->child("action");
		NULL_Fl(pAction, "action not found");

		on();

		return true;
	}

	bool _AutoPhotoBooth::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _AutoPhotoBooth::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateAction();

			m_pT->autoFPSto();
		}
	}

	int _AutoPhotoBooth::check(void)
	{
		IF__(m_vAction.size() <= 0, -1);

		return 0;
	}

	void _AutoPhotoBooth::on(void)
	{
		m_iAction = 0;
		m_bON = true;
		m_bComplete = false;
	}

	void _AutoPhotoBooth::off(void)
	{
		m_bON = false;
	}

	void _AutoPhotoBooth::updateAction(void)
	{
		IF_(check() < 0);
		IF_(!m_bON);

		//wait if still in delay time
		if (m_tResume > 0)
		{
			IF_(m_pT->getTfrom() < m_tResume);
			m_tResume = 0;
		}
	}

}
