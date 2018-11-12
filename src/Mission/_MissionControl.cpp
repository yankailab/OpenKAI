/*
 * _MissionControl.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#include "_MissionControl.h"

namespace kai
{

_MissionControl::_MissionControl()
{
	m_nState = 0;
	m_iState = 0;
	m_iLastState = 0;
}

_MissionControl::~_MissionControl()
{
}

bool _MissionControl::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	//create state instances
	Kiss** pItr = pK->getChildItr();

	m_nState = 0;
	while (pItr[m_nState])
	{
		Kiss* pState = pItr[m_nState];
		IF_F(m_nState >= N_STATE);

		F_ERROR_F(pState->v("state", &m_pStateName[m_nState]));
		m_nState++;
	}

	string startState = "";
	F_ERROR_F(pK->v("startState", &startState));
	m_iState = getStateIdx(startState);
	IF_F(m_iState<0);

	LOG_I(m_pStateName[m_iState]);

	return true;
}

int _MissionControl::getStateIdx(const string& stateName)
{
	for(int i=0;i<m_nState;i++)
	{
		if(m_pStateName[i] == stateName)return i;
	}

	return -1;
}

bool _MissionControl::transit(const string& nextStateName)
{
	int iNext = getStateIdx(nextStateName);

	return transit(iNext);
}

bool _MissionControl::transit(int nextStateIdx)
{
	IF_F(nextStateIdx < 0);
	IF_F(nextStateIdx >= m_nState);
	IF_F(nextStateIdx == m_iState);

	m_iLastState = m_iState;
	m_iState = nextStateIdx;

	LOG_I(m_pStateName[m_iState]);
	return true;
}

int _MissionControl::getCurrentStateIdx(void)
{
	return m_iState;
}

string* _MissionControl::getCurrentStateName(void)
{
	return &m_pStateName[m_iState];
}

int _MissionControl::getLastStateIdx(void)
{
	return m_iLastState;
}

bool _MissionControl::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;

	string msg = *this->getName()+": " + m_pStateName[m_iState];
	pWin->addMsg(&msg);

	return true;
}

}
