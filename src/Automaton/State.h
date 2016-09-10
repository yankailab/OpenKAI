/*
 * State.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_AUTOMATON_STATE_H_
#define OPENKAI_SRC_AUTOMATON_STATE_H_

#include "../Base/common.h"
#include "Transition.h"

#define NUM_STATE_TRANSITION 32

namespace kai
{

class State
{
public:
	State();
	virtual ~State();

	Transition* addTransition(void);
	bool IsValid(void);

public:
	string		m_name;
	int			m_numTransition;
	Transition*	m_pTransition[NUM_STATE_TRANSITION];

};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON_STATE_H_ */
