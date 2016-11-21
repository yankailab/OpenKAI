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

bool ConditionII::init(Kiss* pKiss)
{
	NULL_F(pKiss);

	string condStr;
	F_FATAL_F(pKiss->v("cond", &condStr));

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

	pKiss->v("ptr1", &m_namePtr1);
	pKiss->v("ptr2", &m_namePtr2);

	return true;
}

bool ConditionII::isSatisfied(void)
{
	NULL_F(m_p1);
	NULL_F(m_p2);

	int p1 = (int)(*m_p1);
	int p2 = (int)(*m_p2);

	switch (m_condition)
	{
	case bt:
		if(p1 > p2)return true;
		break;
	case beq:
		if(p1 >= p2)return true;
		break;
	case st:
		if(p1 < p2)return true;
		break;
	case seq:
		if(p1 <= p2)return true;
		break;
	case eq:
		if(p1 == p2)return true;
		break;
	case neq:
		if(p1 != p2)return true;
		break;
	default:
		return false;
		break;
	}

	return false;
}

bool ConditionII::setPtrByName(string* pName, int* ptr)
{
	NULL_F(ptr);
	NULL_F(pName);
	CHECK_F(*pName=="");

	if(*pName==m_namePtr1)m_p1 = ptr;
	if(*pName==m_namePtr2)m_p2 = ptr;

	return true;
}

bool ConditionII::setPtrByName(string* pName, double* ptr)
{
	NULL_F(ptr);
	NULL_F(pName);
	CHECK_F(*pName=="");

	if(*pName==m_namePtr1)m_p1 = (int*)ptr;
	if(*pName==m_namePtr2)m_p2 = (int*)ptr;

	return true;
}


} /* namespace kai */
