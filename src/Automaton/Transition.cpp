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

	m_numCond++;
	return (ConditionII*)*ppC;
}

ConditionFF* Transition::addConditionFF(void)
{
	if(m_numCond >= NUM_TRANSITION_COND)return NULL;

	ConditionBase** ppC = &m_pCond[m_numCond];
	*ppC = new ConditionFF();
	if(*ppC==NULL)return NULL;

	m_numCond++;
	return (ConditionFF*)*ppC;
}

ConditionIC* Transition::addConditionIC(void)
{
	if(m_numCond >= NUM_TRANSITION_COND)return NULL;

	ConditionBase** ppC = &m_pCond[m_numCond];
	*ppC = new ConditionIC();
	if(*ppC==NULL)return NULL;

	m_numCond++;
	return (ConditionIC*)*ppC;
}

bool Transition::setPtrByName(string name, int* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	for(int i=0;i<m_numCond;i++)
	{
		m_pCond[i]->setPtrByName(name,ptr);
	}

	return true;
}

bool Transition::setPtrByName(string name, double* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	for(int i=0;i<m_numCond;i++)
	{
		m_pCond[i]->setPtrByName(name,ptr);
	}

	return true;
}


bool Transition::activate(void)
{
	int i;
	int nSatisfied = 0;

	for(i=0;i<m_numCond;i++)
	{
		if(m_pCond[i]->isSatisfied())
		{
			nSatisfied++;
		}
	}

	if(nSatisfied>=m_numCond)return true;

	return false;
}

} /* namespace kai */
