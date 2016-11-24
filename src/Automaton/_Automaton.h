/*
 * _Automaton.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#ifndef OPENKAI_SRC_AUTOMATON__AUTOMATON_H_
#define OPENKAI_SRC_AUTOMATON__AUTOMATON_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

#define N_STATE 32

namespace kai
{

class _Automaton: public _ThreadBase
{
public:
	_Automaton();
	virtual ~_Automaton();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(Frame* pFrame, vInt4* pTextPos);

	int getStateIdx(string* pStateName);
	int getCurrentStateIdx(void);
	bool transit(string* pNextStateName);
	bool transit(int nextStateIdx);

public:
	int		m_nState;
	int		m_iState;
	string	m_pStateName[N_STATE];

};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON__AUTOMATON_H_ */
