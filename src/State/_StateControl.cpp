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
	for(unsigned int i=0; i<m_vState.size(); i++)
	{
		DEL(m_vState[i].m_pInst);
	}

	m_vState.clear();
}

bool _StateControl::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	Kiss* pCC = pK->child("state");
	IF_T(pCC->empty());

	STATE_INST S;
	int i = 0;
	while (1)
	{
		Kiss* pKs = pCC->child(i++);
		if( pKs->empty())break;

		S.init();

		//Add action modules below

		ADD_STATE (State);
		ADD_STATE (Takeoff);
		ADD_STATE (Loiter);
		ADD_STATE (Waypoint);
		ADD_STATE (Goto);
		ADD_STATE (RTH);
		ADD_STATE (Land);

		//Add action modules above

		IF_Fl(!S.m_pInst, "Unknown state class: "+pKs->m_class);

		pKs->m_pInst = S.m_pInst;
		m_vState.push_back(S);
	}

	IF_F(m_vState.empty());

	for(i=0; i<m_vState.size();i++)
	{
		STATE_INST* pM = &m_vState[i];
		IF_Fl(!pM->m_pInst->init(pM->m_pKiss), pM->m_pKiss->m_name+": init failed");
	}

	string start = "";
	pK->v("start", &start);
	m_iS = getStateIdxByName (start);
	if(m_iS < 0)
		m_iS = 0;

	return true;
}

bool _StateControl::start(void)
{
    IF_F(check()<0);
	return m_pT->start(getUpdate, this);
}

void _StateControl::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		IF_CONT(m_iS >= m_vState.size());
		IF_CONT(m_iS < 0);
		State* pState = m_vState[m_iS].m_pInst;
		if(pState->update())
		{
			transit(pState->m_next);
		}

		m_pT->autoFPSto();
	}
}

void _StateControl::transit(void)
{
	State* pS = m_vState[m_iS].m_pInst;
	NULL_(pS);

	transit(pS->m_next);
}

void _StateControl::transit(const string& mName)
{
	int iNext = getStateIdxByName (mName);
	transit(iNext);
}

void _StateControl::transit(int iS)
{
	IF_( iS < 0);
	IF_( iS >= m_vState.size());
	IF_( iS == m_iS);

	State* pState = m_vState[m_iS].m_pInst;
	pState->reset();

	m_iS = iS;
}

int _StateControl::getStateIdxByName (const string& n )
{
	for(unsigned int i=0; i<m_vState.size(); i++)
	{
		if(((Kiss*)m_vState[i].m_pInst->m_pKiss)->m_name == n )
			return i;
	}

	return -1;
}

State* _StateControl::getState(void)
{
	return m_vState[m_iS].m_pInst;
}

int _StateControl::getStateIdx(void)
{
	return m_iS;
}

string _StateControl::getStateName(void)
{
	string name = ((Kiss*)m_vState[m_iS].m_pInst->m_pKiss)->m_name;
	return name;
}

STATE_TYPE _StateControl::getStateType(void)
{
	State* pMB = getState();

	if(!pMB)
		return state_unknown;

	return pMB->m_type;
}

void _StateControl::draw(void)
{
	this->_ModuleBase::draw();

	addMsg("nState: "+i2str(m_vState.size()),1);
	IF_(m_vState.size() <= 0);

	addMsg("iState: "+i2str(m_iS),1);
	IF_(m_iS < 0);

	State* pMB = m_vState[m_iS].m_pInst;
	addMsg("Current state: " + ((Kiss*)pMB->m_pKiss)->m_name,1);

	if(pMB->type() == state_wp)
		((Waypoint*)pMB)->draw();
	else
		pMB->draw();

}

}
