/*
 * ConditionFF.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOMATON_CONDITIONFF_H_
#define OPENKAI_SRC_AUTOMATON_CONDITIONFF_H_

#include "../Base/common.h"
#include "ConditionBase.h"

namespace kai
{

class ConditionFF: public ConditionBase
{
public:
	ConditionFF();
	virtual ~ConditionFF();

	bool init(Kiss* pKiss);

	bool isSatisfied(void);
	bool setPtrByName(string* pName, int* ptr);
	bool setPtrByName(string* pName, double* ptr);

public:
	double* 	m_p1;
	double* 	m_p2;
};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_CONDITIONFF_H_ */
