/*
 * _Sequencer.h
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Automation__Sequencer_H_
#define OpenKAI_src_Automation__Sequencer_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Actuator/_ActuatorBase.h"

namespace kai
{

struct SEQ_ACTUATOR
{
	_ActuatorBase* m_pA;
	float m_pos;
	float m_speed;

	void init(void)
	{
		m_pA = NULL;
		m_pos = 0.0;
		m_speed = 1.0;
	}

	bool move(void)
	{
		NULL_F(m_pA);
		m_pA->moveTo(m_pos, m_speed);

		return m_pA->bComplete();
	}
};

struct SEQ_ACTION
{
	string	m_name;
	vector<SEQ_ACTUATOR> m_vActuator;
	int		m_dT;	// <0:pause, =0:no delay, >0:delay time

	void init(void)
	{
		m_name = "";
		m_dT = 0;
	}

	SEQ_ACTUATOR* getActuator(const string& name)
	{
		for(int i=0; i<m_vActuator.size(); i++)
		{
			IF_CONT(*m_vActuator[i].m_pA->getName() != name);
			return &m_vActuator[i];
		}

		return NULL;
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
	SEQ_ACTION* getCurrentAction(void);
	SEQ_ACTION* getAction(int iAction);
	SEQ_ACTION* getAction(const string& name);
	int getActionIdx(const string& name);
	void gotoAction(const string& name);
	void updateAction(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Sequencer *) This)->update();
		return NULL;
	}

public:
	vector<SEQ_ACTION> m_vAction;
	int m_iAction;
	int m_iGoAction;
};

}
#endif
