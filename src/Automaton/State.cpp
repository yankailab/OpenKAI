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


} /* namespace kai */
