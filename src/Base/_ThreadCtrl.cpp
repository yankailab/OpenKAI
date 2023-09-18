/*
 * _ThreadCtrl.cpp
 *
 *  Created on: Aug 2, 2023
 *      Author: Kai
 */

#include "_ThreadCtrl.h"

namespace kai
{

	_ThreadCtrl::_ThreadCtrl()
	{
		m_iS = 0;
		m_iTransit = 0;
	}

	_ThreadCtrl::~_ThreadCtrl()
	{
		m_vStates.clear();
	}

	bool _ThreadCtrl::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pKstate = pK->child("state");
		IF_T(pKstate->empty());

		Kiss *pKs;
		int i = 0;
		while (!(pKs = pKstate->child(i++))->empty())
		{
			STATE_THREAD S;
			pKs->v("name", &S.m_name);

			vector<string> vT;
			pKs->a("_Thread", &vT);
			for (string p : vT)
			{
				_Thread *pT = (_Thread *)(pK->getInst(p));
				IF_CONT(!pT);

				S.addThread(pT);
			}

			m_vStates.push_back(S);
		}

		string start = "";
		pK->v("start", &start);
		i = getStateIdxByName(start);
		if (i < 0)
			m_iS = 0;
		else
			m_iS = i;

		return true;
	}

	bool _ThreadCtrl::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;

		return true;
	}

	bool _ThreadCtrl::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _ThreadCtrl::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateStates();

			m_pT->autoFPSto();
		}
	}

	void _ThreadCtrl::updateStates(void)
	{
		while(m_iTransit > 0)
		{
			for(STATE_THREAD sT : m_vStates)
			{
				sT.update(m_iS);
			}

			m_iTransit--;
		}
	}

	void _ThreadCtrl::transitTo(const string &n)
	{
		int iNext = getStateIdxByName(n);
		transitTo(iNext);
	}

	void _ThreadCtrl::transitTo(int iS)
	{
		IF_(iS < 0);
		IF_(iS >= m_vStates.size());
		IF_(iS == m_iS);

		m_iS = iS;
		m_iTransit++;
	}

	int _ThreadCtrl::getStateIdxByName(const string &n)
	{
		for (unsigned int i = 0; i < m_vStates.size(); i++)
		{
			if (m_vStates[i].m_name == n)
				return i;
		}

		return -1;
	}

	int _ThreadCtrl::getCurrentStateIdx(void)
	{
		return m_iS;
	}

	string _ThreadCtrl::getCurrentStateName(void)
	{
		return m_vStates[m_iS].m_name;
	}

	void _ThreadCtrl::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
		IF_(m_vStates.size() <= 0);

		pC->addMsg("iState: " + i2str(m_iS), 1);
		IF_(m_iS < 0);

		pC->addMsg("Current state: " + m_vStates[m_iS].m_name, 1);
	}

}
