#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
	m_pAM = NULL;
	m_timeStamp = 0;
	m_dTime = 0;
	m_vActiveState.clear();
	m_iLastState = 0;
	m_bStateChanged = false;
}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));

	m_vActiveState.clear();
	return true;
}

bool ActionBase::link(void)
{
	IF_F(!this->BASE::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName="";
	F_INFO(pK->v("_Automaton", &iName));
	m_pAM = (_Automaton*) (pK->root()->getChildInstByName(&iName));
	NULL_T(m_pAM);

	Kiss* pAS = pK->o("activeState");
	IF_T(pAS->empty());

	m_vActiveState.clear();
	Kiss** pItr = pAS->getChildItr();
	int i = 0;
	while (pItr[i])
	{
		Kiss* pState = pItr[i++];

		iName="";
		if(pState->v("state", &iName)==false)continue;

		int iState = m_pAM->getStateIdx(&iName);
		if(iState<0)continue;

		m_vActiveState.push_back(iState);
	}

	m_iLastState = m_pAM->getCurrentStateIdx();

	return true;
}

void ActionBase::update(void)
{
	uint64_t newTime = get_time_usec();
	m_dTime = newTime - m_timeStamp;
	m_timeStamp = newTime;

	NULL_(m_pAM);
	int currentState = m_pAM->getCurrentStateIdx();
	if(m_iLastState != currentState)
	{
		m_bStateChanged = true;
		m_iLastState = currentState;
	}
	else
	{
		m_bStateChanged = false;
	}
}

bool ActionBase::isActive(void)
{
	NULL_F(m_pAM);

	int iState = m_pAM->getCurrentStateIdx();
	for (int i : m_vActiveState)
	{
		if(iState == i)return true;
	}

	return false;
}

bool ActionBase::isStateChanged(void)
{
	return m_bStateChanged;
}

bool ActionBase::draw(void)
{
	return this->BASE::draw();
}


}
