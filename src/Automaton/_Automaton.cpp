/*
 * State.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: root
 */

#include "_Automaton.h"

namespace kai
{

_Automaton::_Automaton()
{
	BASE();

	m_nState = 0;
	m_iState = 0;
	m_iLastState = 0;
}

_Automaton::~_Automaton()
{
}

bool _Automaton::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	//create state instances
	Kiss** pItr = pK->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Kiss* pState = pItr[i++];

		IF_F(m_nState >= N_STATE);
		F_ERROR_F(pState->v("state", &m_pStateName[m_nState]));
		m_nState++;
	}

	string startState = "";
	F_ERROR_F(pK->v("startState", &startState));
	m_iState = getStateIdx(&startState);
	IF_F(m_iState<0);

	return true;
}

bool _Automaton::link(void)
{
	IF_F(!this->_ThreadBase::link());
	return true;
}

bool _Automaton::start(void)
{
	return true;
}

int _Automaton::getStateIdx(string* pStateName)
{
	if(pStateName==NULL)return -1;

	for(int i=0;i<m_nState;i++)
	{
		if(m_pStateName[i]==*pStateName)return i;
	}

	return -1;
}

bool _Automaton::transit(string* pNextStateName)
{
	NULL_F(pNextStateName);
	int iNext = getStateIdx(pNextStateName);

	return transit(iNext);
}

bool _Automaton::transit(int nextStateIdx)
{
	IF_F(nextStateIdx < 0);
	IF_F(nextStateIdx >= m_nState);
	IF_F(nextStateIdx == m_iState);

	m_iLastState = m_iState;
	m_iState = nextStateIdx;

	LOG_I("Transit: "<<m_pStateName[m_iState]);
	return true;
}

int _Automaton::getCurrentStateIdx(void)
{
	return m_iState;
}

string* _Automaton::getCurrentStateName(void)
{
	return &m_pStateName[m_iState];
}

int _Automaton::getLastStateIdx(void)
{
	return m_iLastState;
}

bool _Automaton::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;

	string msg = *this->getName()+": " + m_pStateName[m_iState];
	pWin->addMsg(&msg);

	return true;
}

}
