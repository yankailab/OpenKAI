/*
 * ConditionIC.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOMATON_CONDITIONIC_H_
#define OPENKAI_SRC_AUTOMATON_CONDITIONIC_H_

#include "../Base/common.h"
#include "ConditionBase.h"

namespace kai
{

class ConditionIC: public ConditionBase
{
public:
	ConditionIC();
	virtual ~ConditionIC();

	bool isSatisfied(void);
	bool setPtrByName(string name, int* ptr);
	bool setPtrByName(string name, double* ptr);

public:
	int* 	m_p1;
	int 	m_const;
};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_CONDITIONIC_H_ */
