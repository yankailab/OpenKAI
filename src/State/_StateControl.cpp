/*
 * _StateControl.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#include "_StateControl.h"

namespace kai
{

	_StateControl::_StateControl()
	{
		m_iS = 0;
	}

	_StateControl::~_StateControl()
	{
		for (unsigned int i = 0; i < m_vpState.size(); i++)
		{
			DEL(m_vpState[i]);
		}

		m_vpState.clear();
	}

	int _StateControl::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pKstate = pK->child("state");
		IF__(pKstate->empty(), OK_OK);

		int i = 0;
		while (1)
		{
			Kiss *pKs = pKstate->child(i++);
			if (pKs->empty())
				break;

			// Add state below

			ADD_STATE(StateBase);

			// Add state above

			LOG_E("Unknown state class: " + pKs->getClass());
			return OK_ERR_NOT_FOUND;
		}

		IF__(m_vpState.empty(), OK_ERR_NOT_FOUND);

		string start = "";
		pK->v("start", &start);
		i = getStateIdxByName(start);
		if (i < 0)
			m_iS = 0;
		else
			m_iS = i;

		return OK_OK;
	}

	int _StateControl::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		for (int i = 0; i < m_vpState.size(); i++)
		{
			CHECK_(m_vpState[i]->link());
		}

		return OK_OK;
	}

	int _StateControl::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _StateControl::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			IF_CONT(m_iS >= m_vpState.size());
			IF_CONT(m_iS < 0);

			StateBase *pS = getCurrentState();
			if (pS)
			{
				pS->update();
				pS->updateModules();

				if (pS->bComplete())
					transit(pS->getNext());
			}

		}
	}

	void _StateControl::transit(void)
	{
		StateBase *pS = m_vpState[m_iS];
		NULL_(pS);

		transit(pS->getNext());
	}

	void _StateControl::transit(const string &mName)
	{
		int iNext = getStateIdxByName(mName);
		transit(iNext);
	}

	void _StateControl::transit(int iS)
	{
		IF_(iS < 0);
		IF_(iS >= m_vpState.size());
		IF_(iS == m_iS);

		StateBase *pS = m_vpState[m_iS];
		pS->reset();

		m_iS = iS;
	}

	int _StateControl::getStateIdxByName(const string &n)
	{
		for (int i = 0; i < m_vpState.size(); i++)
		{
			if (m_vpState[i]->getName() == n)
				return i;
		}

		return -1;
	}

	StateBase *_StateControl::getCurrentState(void)
	{
		return m_vpState[m_iS];
	}

	int _StateControl::getCurrentStateIdx(void)
	{
		return m_iS;
	}

	string _StateControl::getCurrentStateName(void)
	{
		return m_vpState[m_iS]->getName();
	}

	STATE_TYPE _StateControl::getCurrentStateType(void)
	{
		StateBase *pS = getCurrentState();
		IF__(!pS, state_unknown);

		return pS->getType();
	}

	void _StateControl::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("nState: " + i2str(m_vpState.size()), 0);
		IF_(m_vpState.size() <= 0);

		pC->addMsg("iState: " + i2str(m_iS), 1);
		IF_(m_iS < 0);

		StateBase *pS = getCurrentState();
		pC->addMsg("Current state: " + getCurrentStateName(), 1);
		pS->console(pConsole);
	}

}
