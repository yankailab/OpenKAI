/*
 * ConditionII.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#include "ConditionII.h"

namespace kai
{

ConditionII::ConditionII()
{
	ConditionBase();
	m_p1 = NULL;
	m_p2 = NULL;
}

ConditionII::~ConditionII()
{
}

bool ConditionII::isSatisfied(void)
{
	if(m_p1==NULL)return false;
	if(m_p2==NULL)return false;

	switch (m_condition)
	{
	case bt:
		if(*m_p1 > *m_p2)return true;
		break;
	case beq:
		if(*m_p1 >= *m_p2)return true;
		break;
	case st:
		if(*m_p1 < *m_p2)return true;
		break;
	case seq:
		if(*m_p1 <= *m_p2)return true;
		break;
	case eq:
		if(*m_p1 == *m_p2)return true;
		break;
	case neq:
		if(*m_p1 != *m_p2)return true;
		break;
	default:
		return false;
		break;
	}

	return false;
}

} /* namespace kai */
