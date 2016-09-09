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
#include "State.h"

#define NUM_STATE 16

namespace kai
{

class _Automaton: public _ThreadBase
{
public:
	_Automaton();
	virtual ~_Automaton();

	bool init(JSON* pJson, string stateName);
	bool start(void);

	State* addState(void);
	bool checkDiagram(void);

public:
	void updateAll(void);

	int		m_iMyState;
	int		m_numState;
	State	m_pState[NUM_STATE];


	//Thread
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Automaton *) This)->update();
		return NULL;
	}

};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOMATON__AUTOMATON_H_ */
