/*
 * ConditionIC.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#include "ConditionIC.h"

namespace kai
{

ConditionIC::ConditionIC()
{
	ConditionBase();
	m_p1 = NULL;
	m_const = 0;
}

ConditionIC::~ConditionIC()
{
}

bool ConditionIC::isSatisfied(void)
{
	if(m_p1==NULL)return false;

	int p1 = (int)(*m_p1);

	switch (m_condition)
	{
	case bt:
		if(p1 > m_const)return true;
		break;
	case beq:
		if(p1 >= m_const)return true;
		break;
	case st:
		if(p1 < m_const)return true;
		break;
	case seq:
		if(p1 <= m_const)return true;
		break;
	case eq:
		if(p1 == m_const)return true;
		break;
	case neq:
		if(p1 != m_const)return true;
		break;
	default:
		return false;
		break;
	}

	return false;
}

bool ConditionIC::setPtrByName(string name, int* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	if(name==m_namePtr1)m_p1 = ptr;
	if(name==m_namePtr2)m_const = *ptr;

	return true;
}

bool ConditionIC::setPtrByName(string name, double* ptr)
{
	if(ptr==NULL)return false;
	if(name=="")return false;

	if(name==m_namePtr1)m_p1 = (int*)ptr;
	if(name==m_namePtr2)m_const = (int)(*ptr);

	return true;
}

} /* namespace kai */
