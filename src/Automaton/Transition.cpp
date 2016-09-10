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
	m_transitToID = -1;
	m_numCond = 0;
	m_flagSatisfied = 0;
}

Transition::~Transition()
{
}

ConditionII* Transition::addConditionII(void)
{
	if(m_numCond >= NUM_TRANSITION_COND)return NULL;

	ConditionBase** ppC = &m_pCond[m_numCond];
	*ppC = new ConditionII();
	if(*ppC==NULL)return NULL;

	m_flagSatisfied |= (1<<m_numCond);
	m_numCond++;

	return (ConditionII*)*ppC;
}

bool Transition::activate(void)
{
	int i;
	int flag = 0;

	for(i=0;i<m_numCond;i++)
	{
		if(m_pCond[i]->isSatisfied())
		{
			flag |= (1<<i);
		}
	}

	if(flag==m_flagSatisfied)return true;

	return false;
}

} /* namespace kai */
