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

bool ConditionIC::init(Config* pConfig)
{
	NULL_F(pConfig);

	string condStr;
	F_FATAL_F(pConfig->v("cond", &condStr));

	if (condStr == "bt")
		m_condition = bt;
	else if (condStr == "beq")
		m_condition = beq;
	else if (condStr == "st")
		m_condition = st;
	else if (condStr == "seq")
		m_condition = seq;
	else if (condStr == "eq")
		m_condition = eq;
	else if (condStr == "neq")
		m_condition = neq;
	else
		LOG(FATAL)<<"Unknown condition";

	pConfig->v("ptr1", &m_namePtr1);
	pConfig->v("constI", &m_const);

	return true;
}

bool ConditionIC::isSatisfied(void)
{
	NULL_F(m_p1);

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

bool ConditionIC::setPtrByName(string* pName, int* ptr)
{
	NULL_F(ptr);
	NULL_F(pName);
	CHECK_F(*pName=="");

	if(*pName==m_namePtr1)m_p1 = ptr;
	if(*pName==m_namePtr2)m_const = *ptr;

	return true;
}

bool ConditionIC::setPtrByName(string* pName, double* ptr)
{
	NULL_F(ptr);
	NULL_F(pName);
	CHECK_F(*pName=="");

	if(*pName==m_namePtr1)m_p1 = (int*)ptr;
	if(*pName==m_namePtr2)m_const = (int)(*ptr);

	return true;
}

} /* namespace kai */
