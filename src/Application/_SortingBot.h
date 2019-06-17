/*
 * _SortingBot.h
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__SortingBot_H_
#define OpenKAI_src_Application__SortingBot_H_

#include "../Base/common.h"
#include "../Detector/_DetectorBase.h"
#include "_Sequencer.h"

#define SB_N_CLASS 16

namespace kai
{

struct SB_ARMSET
{
	_Sequencer*	m_pSeq;
	uint64_t	m_classFlag;
	vFloat2		m_rGripX;	//grip region X, target bb midX will be re-mapped into this region
	vFloat2		m_rGripY;	//grip region Y
	vFloat2		m_rGripZ;	//vertical
	int			m_iActuatorX;
	int			m_iActuatorZ;
	int			m_iROI;

	void init(void)
	{
		m_pSeq = NULL;
		m_classFlag = 0;
		m_rGripX.init();
		m_rGripY.init();
		m_rGripZ.init();
		m_iActuatorX = 0;
		m_iActuatorZ = 1;
		m_iROI = 0;
	}

	bool bClass(int iClass)
	{
		return m_classFlag & (1 << iClass);
	}

	string getCurrentActionName(void)
	{
		return m_pSeq->getCurrentActionName();
	}

	SEQUENCER_ACTION* getAction(const string& name)
	{
		return m_pSeq->getAction(name);
	}

	SEQUENCER_ACTION* getAction(int iAction)
	{
		return m_pSeq->getAction(iAction);
	}
};

class _SortingBot: public _ThreadBase
{
public:
	_SortingBot(void);
	virtual ~_SortingBot();

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
		((_SortingBot *) This)->update();
		return NULL;
	}

public:
	_DetectorBase* m_pSW;
	_DetectorBase* m_pC;

	deque<OBJECT> m_vTarget;
	vector<SB_ARMSET> m_vArmSet;

	int m_nClass;
	float m_pDropPos[SB_N_CLASS];
	float m_cSpeed; //conveyer speed
	float m_cLen;	//conveyer length
	float m_minOverlap;
};

}
#endif
