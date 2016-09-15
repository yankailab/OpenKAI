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
#include "../Stream/Frame.h"
#include "State.h"

#define NUM_STATE 32

namespace kai
{

class _Automaton: public _ThreadBase
{
public:
	_Automaton();
	virtual ~_Automaton();

	bool init(Config* pConfig, string stateName);
	bool start(void);

	State* addState(void);
	bool setState(int iState);
	bool checkDiagram(void);

	bool setPtrByName(string name, int* ptr);
	bool setPtrByName(string name, double* ptr);

	bool draw(Frame* pFrame, iVec4* pTextPos);

public:
	void updateAll(void);

	int		m_iState;
	int		m_numState;
	State*	m_pState[NUM_STATE];

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
