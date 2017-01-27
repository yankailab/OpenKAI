#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
	m_pAM = NULL;
	m_timeStamp = 0;
	m_dTime = 0;
	m_vActiveState.clear();
}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(void* pKiss)
{
	CHECK_F(!this->BASE::init(pKiss));

	m_vActiveState.clear();
	return true;
}

bool ActionBase::link(void)
{
	CHECK_F(!this->BASE::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName="";
	F_INFO(pK->v("_Automaton", &iName));
	m_pAM = (_Automaton*) (pK->root()->getChildInstByName(&iName));
	NULL_T(m_pAM);

	iName = "activeState";
	Kiss* pAS = pK->getChildByName(&iName);
	NULL_T(pAS);

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

	return true;
}

void ActionBase::update(void)
{
	uint64_t newTime = get_time_usec();
	m_dTime = newTime - m_timeStamp;
	m_timeStamp = newTime;
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

bool ActionBase::draw(void)
{
	return this->BASE::draw();
}


}
