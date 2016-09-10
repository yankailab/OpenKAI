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
	m_condition = DEFAULT;
}

ConditionBase::~ConditionBase()
{
}

bool ConditionBase::isSatisfied(void)
{
	return false;
}

} /* namespace kai */
