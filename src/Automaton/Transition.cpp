/*
 * Transition.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#include "Transition.h"

namespace kai
{

Transition::Transition()
{
	m_nCond = 0;
	m_strToState = "";
	m_pToState = NULL;
}

Transition::~Transition()
{
}

bool Transition::init(Kiss* pKiss)
{
	NULL_F(pKiss);
	F_FATAL_F(pKiss->v("toState", &m_strToState));

	pKiss->m_pInst = (void*)this;

	Kiss** pItr = pKiss->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Kiss* pCond = pItr[i];
		i++;

		bool bInst = false;
		F_INFO(pCond->v("bInst", &bInst));
		if (!bInst)continue;
		CHECK_F(m_nCond >= N_COND);

		if(pCond->m_class=="ConditionII")
		{
			F_ERROR_F(addCondition<ConditionII>(pCond));
		}
		else if(pCond->m_class=="ConditionIC")
		{
			F_ERROR_F(addCondition<ConditionIC>(pCond));
		}
		else if(pCond->m_class=="ConditionFF")
		{
			F_ERROR_F(addCondition<ConditionFF>(pCond));
		}
		else
		{
			LOG(FATAL)<<"Unknown condition";
		}
	}

	return true;
}

bool Transition::link(Kiss* pKiss)
{
	NULL_F(pKiss);

	m_pToState = pKiss->getChildInstByName(&m_strToState);
	NULL_F(m_pToState);

	return true;
}

template <typename T> bool Transition::addCondition(Kiss* pKiss)
{
	CHECK_N(m_nCond >= N_COND);

	ConditionBase** ppC = &m_pCond[m_nCond];
	m_nCond++;
	*ppC = new T();
	NULL_N(*ppC);

	F_FATAL_F(((T*)*ppC)->init(pKiss));
	pKiss->m_pInst = (void*)(*ppC);

	return true;
}

bool Transition::setPtrByName(string name, int* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	for(int i=0;i<m_nCond;i++)
	{
		m_pCond[i]->setPtrByName(name,ptr);
	}

	return true;
}

bool Transition::setPtrByName(string name, double* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	for(int i=0;i<m_nCond;i++)
	{
		m_pCond[i]->setPtrByName(name,ptr);
	}

	return true;
}


bool Transition::activate(void)
{
	int i;
	int nSatisfied = 0;

	for(i=0;i<m_nCond;i++)
	{
		if(m_pCond[i]->isSatisfied())
		{
			nSatisfied++;
		}
	}

	if(nSatisfied>=m_nCond)return true;

	return false;
}

} /* namespace kai */
