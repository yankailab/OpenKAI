/*
 * Transition.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOMATON_TRANSITION_H_
#define OPENKAI_SRC_AUTOMATON_TRANSITION_H_

#include "../Base/common.h"

#define NUM_TRANSITION_COND 32

namespace kai
{

enum COMP_COND
{
	DEFAULT,bt,beq,st,seq,eq,neq
};

struct TRANSITION_COND
{
	int* 	m_pI1;
	double* m_pF1;

	int* 	m_pI2;
	double* m_pF2;
	int		m_pIConst2;
	double	m_pFConst2;

	string			m_namePtr1;
	string			m_namePtr2;
	COMP_COND		m_condition;
};

class Transition
{
public:
	Transition();
	virtual ~Transition();

	TRANSITION_COND* addCondition(void);
	bool IsValid(void);

public:
	int					m_transitToID;
	int					m_numCond;
	TRANSITION_COND 	m_pCond[NUM_TRANSITION_COND];
	int					m_flagSatisfied;
};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_TRANSITION_H_ */
