/*
 * State.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: root
 */

#include "State.h"

namespace kai
{

State::State()
{
	m_name = "";
	m_numTransition = 0;
}

State::~State()
{
}

Transition* State::addTransition(void)
{
	if(m_numTransition >= NUM_STATE_TRANSITION)return NULL;
	Transition** ppT = &m_pTransition[m_numTransition];
	*ppT = new Transition();
	if(*ppT==NULL)return NULL;

	m_numTransition++;
	return *ppT;
}

bool State::IsValid(void)
{

	return true;
}

int State::Transit(void)
{
	for(int i=0; i<m_numTransition; i++)
	{
		Transition* pT = m_pTransition[i];
		if(pT->activate())
		{
			return pT->m_transitToID;
		}
	}

	return -1;
}

bool State::setPtrByName(string name, int* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	for(int i=0;i<m_numTransition;i++)
	{
		m_pTransition[i]->setPtrByName(name,ptr);
	}

	return true;
}

bool State::setPtrByName(string name, double* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	for(int i=0;i<m_numTransition;i++)
	{
		m_pTransition[i]->setPtrByName(name,ptr);
	}

	return true;
}


} /* namespace kai */
