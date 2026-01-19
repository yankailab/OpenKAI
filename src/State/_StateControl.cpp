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

	bool _StateControl::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		const json &jS = j.at("states");
		IF__(!jS.is_object(), true);

		for (auto it = jS.begin(); it != jS.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			StateBase *pB = new StateBase();
			NULL_F(pB);
			if (!pB->init(Ji))
			{
				LOG_E("Init failed: " + pB->getName());
				DEL(pB);
				return false;
			}

			m_vpState.push_back(pB);
		}

		IF__(m_vpState.empty(), true);

		string start = "";
		jVar(j, "start", start);
		int i = getStateIdxByName(start);
		if (i < 0)
			m_iS = 0;
		else
			m_iS = i;

		return true;
	}

	bool _StateControl::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		for (int i = 0; i < m_vpState.size(); i++)
		{
			IF_F(!m_vpState[i]->link(m_vpState[i]->getName(), pM));
		}

		return true;
	}

	bool _StateControl::start(void)
	{
		NULL_F(m_pT);
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
