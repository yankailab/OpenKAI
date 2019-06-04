/*
 * _Sequencer.h
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__Sequencer_H_
#define OpenKAI_src_Application__Sequencer_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Actuator/_ActuatorBase.h"

#define SQ_N_ACTUATOR 16

namespace kai
{

struct SEQUENCER_ACTION
{
	string	m_name;
	float	m_pNpos[SQ_N_ACTUATOR];
	int		m_nA;
	int		m_dT;	// <0:pause, =0:no delay, >0:delay time

	void init(void)
	{
		m_name = "";
		m_nA = 0;
		m_dT = 0;

		for(int i=0; i<SQ_N_ACTUATOR; i++)
			m_pNpos[i] = -1.0;
	}
};

class _Sequencer: public _ThreadBase
{
public:
	_Sequencer(void);
	virtual ~_Sequencer();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

	string getCurrentActionName(void);
	SEQUENCER_ACTION* getCurrentAction(void);
	SEQUENCER_ACTION* getAction(int iAction);
	SEQUENCER_ACTION* getAction(const string& name);
	void updateAction(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Sequencer *) This)->update();
		return NULL;
	}

public:
	_ActuatorBase* m_ppActuator[SQ_N_ACTUATOR];
	int	m_nActuator;
	vector<SEQUENCER_ACTION> m_vAction;
	int m_iAction;
};

}
#endif
