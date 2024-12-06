/*
 * _Sequencer.cpp
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#include "../RobotArm/_Sequencer.h"

namespace kai
{

	_Sequencer::_Sequencer()
	{
		m_bON = false;
		m_iAction = 0;
		m_tResume = 0;
		m_iGoAction = -1;
		m_bComplete = false;
	}

	_Sequencer::~_Sequencer()
	{
	}

	int _Sequencer::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pAction = pK->child("action");
		NULL__(pAction, OK_ERR_NOT_FOUND);

		int i = 0;
		while (1)
		{
			Kiss *pActionI = pAction->child(i++);
			if (pActionI->empty())
				break;

			SEQ_ACTION sa;
			sa.init();
			pActionI->v("name", &sa.m_name);
			pActionI->v("dT", &sa.m_dT);

			int j = 0;
			while (1)
			{
				Kiss *pActuatorI = pActionI->child(j++);
				if (pActuatorI->empty())
					break;

				SEQ_ACTUATOR aA;
				aA.init();

				string n = "";
				pActuatorI->v("_ActuatorBase", &n);
				aA.m_pA = (_ActuatorBase *)(pK->findModule(n));
				IF_CONT(!aA.m_pA);

				pActuatorI->v("pos", &aA.m_vPos);
				pActuatorI->v("speed", &aA.m_vSpeed);

				sa.m_vActuator.push_back(aA);
			}

			m_vAction.push_back(sa);
		}

		on();

		return OK_OK;
	}

	int _Sequencer::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _Sequencer::check(void)
	{
		IF__(m_vAction.size() <= 0, OK_ERR_INVALID_VALUE);

		return OK_OK;
	}

	void _Sequencer::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateAction();

		}
	}

	void _Sequencer::on(void)
	{
		m_iAction = 0;
		m_tResume = 0;
		m_bON = true;
		m_bComplete = false;
	}

	void _Sequencer::off(void)
	{
		m_bON = false;
	}

	void _Sequencer::updateAction(void)
	{
		IF_(check() != OK_OK);
		IF_(!m_bON);

		//wait if still in delay time
		if (m_tResume > 0)
		{
			IF_(m_pT->getTfrom() < m_tResume);
			m_tResume = 0;
		}

		//decide next dest
		if (m_iGoAction >= 0)
		{
			m_iAction = m_iGoAction;
			m_iGoAction = -1;
		}
		else if (m_bComplete)
		{
			m_iAction++;
			if (m_iAction >= m_vAction.size())
				m_iAction = 0;
		}

		m_bComplete = false;

		//move arms to dest
		unsigned int i;
		unsigned int nComplete = 0;
		SEQ_ACTION *pAction = &m_vAction[m_iAction];
		for (i = 0; i < pAction->m_vActuator.size(); i++)
		{
			SEQ_ACTUATOR *pSA = &pAction->m_vActuator[i];
			if (pSA->move())
				nComplete++;
		}
		IF_(nComplete < pAction->m_vActuator.size());

		m_bComplete = true;

		//delay time after motion complete
		if (pAction->m_dT > 0)
			m_tResume = m_pT->getTfrom() + pAction->m_dT;
		else if (pAction->m_dT < 0)
			m_tResume = UINT64_MAX;
	}

	SEQ_ACTION *_Sequencer::getAction(int iAction)
	{
		IF_N(iAction >= m_vAction.size());
		return &m_vAction[iAction];
	}

	SEQ_ACTION *_Sequencer::getAction(const string &name)
	{
		for (int i = 0; i < m_vAction.size(); i++)
		{
			SEQ_ACTION *pA = &m_vAction[i];
			IF_CONT(pA->m_name != name);
			return pA;
		}

		return NULL;
	}

	SEQ_ACTION *_Sequencer::getCurrentAction(void)
	{
		return &m_vAction[m_iAction];
	}

	string _Sequencer::getCurrentActionName(void)
	{
		return m_vAction[m_iAction].m_name;
	}

	int _Sequencer::getActionIdx(const string &name)
	{
		for (int i = 0; i < m_vAction.size(); i++)
		{
			SEQ_ACTION *pA = &m_vAction[i];
			IF_CONT(pA->m_name != name);
			return i;
		}

		return -1;
	}

	void _Sequencer::gotoAction(const string &name)
	{
		m_iGoAction = getActionIdx(name);
	}

}
