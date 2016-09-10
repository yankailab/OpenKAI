/*
 * Transition.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOMATON_TRANSITION_H_
#define OPENKAI_SRC_AUTOMATON_TRANSITION_H_

#include "../Base/common.h"
#include "ConditionII.h"

#define NUM_TRANSITION_COND 32

namespace kai
{

class Transition
{
public:
	Transition();
	virtual ~Transition();

	ConditionII* addConditionII(void);
	ConditionBase* addConditionIF(void);
	ConditionBase* addConditionFF(void);
	ConditionBase* addConditionIC(void);

	bool activate(void);
	bool isValid(void);

public:
	int					m_transitToID;
	int					m_numCond;
	ConditionBase*	 	m_pCond[NUM_TRANSITION_COND];
	int					m_flagSatisfied;
};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_TRANSITION_H_ */
