/*
 * State.h
 *
 *  Created on: Aug 27, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_AUTOPILOT__STATE_H_
#define OPENKAI_SRC_AUTOPILOT__STATE_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

#define NUM_STATE 128
#define NUM_STATE_TRANSITION 32
#define NUM_STATE_TRANSITION_PARAMS 32
#define NUM_STATE_TRANSITION_COND 32

namespace kai
{

enum TRANSITION_COND
{
	DEFAULT,NONE,AND,OR,bt,beq,st,seq,eq,neq
};

struct STATE_TRANSITION_COND
{
	double* 		m_pParam1;
	double*			m_pParam2;
	TRANSITION_COND m_condition;
	bool			m_bSatisfied;
};

struct STATE_TRANSITION
{
	int 	m_transitToIdx;
	int		m_flagSatiefied;
	TRANSITION_COND	m_condition;

	int		m_numCond;
	STATE_TRANSITION_COND 	m_pCond[NUM_STATE_TRANSITION_COND];

	int		m_numParam1;
	double	m_pParam1[NUM_STATE_TRANSITION_PARAMS];
	int		m_numParam2;
	double	m_pParam2[NUM_STATE_TRANSITION_PARAMS];
};

struct STATE
{
	string	m_name;

	int		m_numTransition;
	STATE_TRANSITION m_pTransition[NUM_STATE_TRANSITION];
};


class _State: public _ThreadBase
{
public:
	_State();
	virtual ~_State();

	bool init(JSON* pJson, string stateName);
	bool start(void);

	STATE* addState(string name);
	STATE_TRANSITION* addStateTransition(STATE* pState);
	STATE_TRANSITION_COND* addStateTransitionCond(STATE_TRANSITION* pStateTransition);

public:
	void updateAll(void);

	int m_stateIdx;
	int m_numState;
	STATE m_pState[NUM_STATE];

	//Thread
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_State *) This)->update();
		return NULL;
	}

};

} /* namespace kai */

#endif /* OPENKAI_SRC_AUTOPILOT__STATE_H_ */
