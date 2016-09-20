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
	m_iToState = -1;
	m_nCond = 0;
}

Transition::~Transition()
{
}

bool Transition::init(Config* pConfig)
{
	NULL_F(pConfig);
	F_FATAL_F(pConfig->v("toState", &m_iToState));

	Config** pItr = pConfig->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Config* pCond = pItr[i];
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

template <typename T> bool Transition::addCondition(Config* pConfig)
{
	CHECK_N(m_nCond >= N_COND);

	ConditionBase** ppC = &m_pCond[m_nCond];
	m_nCond++;
	*ppC = new T();
	NULL_N(*ppC);

	F_FATAL_F(((T*)*ppC)->init(pConfig));
	pConfig->m_pInst = (void*)(*ppC);

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
