/*
 * _ClassifierBot.h
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__ClassifierBot_H_
#define OpenKAI_src_Application__ClassifierBot_H_

#include "../Base/common.h"
#include "../Detector/_DetectorBase.h"
#include "_Sequencer.h"

#define CB_N_CLASS 16

namespace kai
{

struct CBOT_TARGET
{
	int m_iClass;
	vFloat4 m_pos;
	uint64_t m_tStamp;

	void init(void)
	{
		m_iClass = -1;
		m_pos.init();
		m_tStamp = 0;
	}
};

struct CBOT_ARM
{
	_Sequencer* m_pArm;
	uint64_t m_classFlag;
	float m_tGrip; //time taken from standby to grip in sec
	int m_iActionGripStandby;
	int m_iActionDrop;

	void init(void)
	{
		m_pArm = NULL;
		m_classFlag = 0;
		m_tGrip = 0.0;
		m_iActionGripStandby = -1;
		m_iActionDrop = -1;
	}
};

class _ClassifierBot: public _ThreadBase
{
public:
	_ClassifierBot(void);
	virtual ~_ClassifierBot();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	void updateAction(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ClassifierBot *) This)->update();
		return NULL;
	}

public:
	_DetectorBase* m_pDet;
	vector<CBOT_TARGET> m_vTarget;
	vector<CBOT_ARM> m_vArm;

	int m_nClass;
	float m_pDropPos[CB_N_CLASS];
	float m_speed; //conveyer speed m/s
};

}
#endif
