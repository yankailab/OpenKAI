/*
 * _ClassifierBot.h
 *
 *  Created on: May 28, 2019
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
	vFloat4 m_bb;
	uint64_t m_tStamp;

	void init(void)
	{
		m_iClass = -1;
		m_bb.init();
		m_tStamp = 0;
	}
};

struct CBOT_ARMSET
{
	_Sequencer*	m_pSeq;
	uint64_t	m_classFlag;
	vFloat2	m_rGripX;	//grippable region X, target bb midX will be remapped into this region
	vFloat2	m_rGripY;	//grippable region Y
	int		m_iActionGripStandby;
	int		m_iActionDrop;
	int		m_iActuatorX;
	bool	m_bTarget;

	void init(void)
	{
		m_pSeq = NULL;
		m_classFlag = 0;
		m_rGripX.init();
		m_rGripY.init();
		m_iActionGripStandby = -1;
		m_iActionDrop = -1;
		m_iActuatorX = 0;
		m_bTarget = false;
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
	void updateTarget(void);
	void updateArmset(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ClassifierBot *) This)->update();
		return NULL;
	}

public:
	_DetectorBase* m_pDet;
	vector<CBOT_TARGET> m_vTarget;
	vector<CBOT_ARMSET> m_vArmSet;

	int m_nClass;
	float m_pDropPos[CB_N_CLASS];
	float m_speed; //conveyer speed m/s
	float m_bbOverlap;
	float m_cLen;
	float m_kD;
};

}
#endif
