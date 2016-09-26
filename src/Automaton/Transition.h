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
#include "ConditionFF.h"
#include "ConditionIC.h"

#define N_COND 32

namespace kai
{

class Transition
{
public:
	Transition();
	virtual ~Transition();

	bool init(Config* pConfig);
	bool link(Config* pConfig);

	template <typename T> bool addCondition(Config* pConfig);
	bool activate(void);
	bool isValid(void);

	bool setPtrByName(string name, int* ptr);
	bool setPtrByName(string name, double* ptr);

public:
	void*				m_pToState;
	string				m_strToState;
	int					m_nCond;
	ConditionBase*	 	m_pCond[N_COND];
};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_TRANSITION_H_ */
