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
	m_nTransition = 0;
}

State::~State()
{
}

bool State::init(Config* pConfig)
{
	NULL_F(pConfig);
	F_FATAL_F(pConfig->v("name", &m_name));

	pConfig->m_pInst = (void*)this;

	Config** pItr = pConfig->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Config* pT = pItr[i];
		i++;

		bool bInst = false;
		F_INFO(pT->v("bInst", &bInst));
		if (!bInst)continue;
		if (pT->m_class != "Transition")continue;
		CHECK_F(m_nTransition >= N_TRANSITION);

		Transition** ppT = &m_pTransition[m_nTransition];
		m_nTransition++;

		*ppT = new Transition();
		F_ERROR_F((*ppT)->init(pT));
	}

	return true;
}

bool State::link(Config* pConfig)
{
	NULL_F(pConfig);

	for(int i=0;i<m_nTransition;i++)
	{
		F_ERROR_F(m_pTransition[i]->link(pConfig));
	}

	return true;
}

Transition* State::addTransition(void)
{
	if(m_nTransition >= N_TRANSITION)return NULL;
	Transition** ppT = &m_pTransition[m_nTransition];
	*ppT = new Transition();
	if(*ppT==NULL)return NULL;

	m_nTransition++;
	return *ppT;
}

bool State::IsValid(void)
{

	return true;
}

State* State::Transit(void)
{
	for(int i=0; i<m_nTransition; i++)
	{
		Transition* pT = m_pTransition[i];
		if(pT->activate())
		{
			return (State*)(pT->m_pToState);
		}
	}

	return NULL;
}

bool State::setPtrByName(string name, int* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	for(int i=0;i<m_nTransition;i++)
	{
		m_pTransition[i]->setPtrByName(name,ptr);
	}

	return true;
}

bool State::setPtrByName(string name, double* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	for(int i=0;i<m_nTransition;i++)
	{
		m_pTransition[i]->setPtrByName(name,ptr);
	}

	return true;
}


} /* namespace kai */
