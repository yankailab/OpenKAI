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
	bt,beq,st,seq,eq,neq
};

class ConditionBase
{
public:
	ConditionBase();
	virtual ~ConditionBase();

	virtual bool isSatisfied(void);
	virtual bool setPtrByName(string name, int* ptr);
	virtual bool setPtrByName(string name, double* ptr);

public:
	string			m_namePtr1;
	string			m_namePtr2;
	COMP_COND		m_condition;

};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_CONDITIONBASE_H_ */
