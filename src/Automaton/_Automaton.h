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

	bool checkDiagram(void);

	bool setPtrByName(string name, int* ptr);
	bool setPtrByName(string name, double* ptr);

	bool draw(Frame* pFrame, vInt4* pTextPos);

public:
	void updateAll(void);

	int		m_nState;
//	State*	m_pState[N_STATE];
//	State*	m_pMyState;

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
