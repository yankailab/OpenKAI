/*
 * ConditionBase.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOMATON_CONDITIONBASE_H_
#define OPENKAI_SRC_AUTOMATON_CONDITIONBASE_H_

#include "../Base/common.h"

namespace kai
{

enum COMP_COND
{
	DEFAULT,bt,beq,st,seq,eq,neq
};

class ConditionBase
{
public:
	ConditionBase();
	virtual ~ConditionBase();

	virtual bool isSatisfied(void);

public:
	string			m_namePtr1;
	string			m_namePtr2;
	COMP_COND		m_condition;

};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_CONDITIONBASE_H_ */
