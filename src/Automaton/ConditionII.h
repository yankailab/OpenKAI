/*
 * ConditionII.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOMATON_CONDITIONII_H_
#define OPENKAI_SRC_AUTOMATON_CONDITIONII_H_

#include "../Base/common.h"
#include "ConditionBase.h"

namespace kai
{

class ConditionII: public ConditionBase
{
public:
	ConditionII();
	virtual ~ConditionII();

	bool init(Config* pConfig);

	bool isSatisfied(void);
	bool setPtrByName(string* pName, int* ptr);
	bool setPtrByName(string* pName, double* ptr);

public:
	int* 	m_p1;
	int* 	m_p2;
};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_CONDITIONII_H_ */
