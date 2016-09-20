/*
 * ConditionBase.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#include "ConditionBase.h"

namespace kai
{

ConditionBase::ConditionBase()
{
	m_namePtr1 = "";
	m_namePtr2 = "";
	m_condition = bt;
}

ConditionBase::~ConditionBase()
{
}

bool ConditionBase::isSatisfied(void)
{
	return false;
}

bool ConditionBase::setPtrByName(string name, int* ptr)
{
	return false;
}

bool ConditionBase::setPtrByName(string name, double* ptr)
{
	return false;
}

} /* namespace kai */
