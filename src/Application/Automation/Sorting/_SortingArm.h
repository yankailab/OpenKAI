/*
 * _SortingArm.h
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Automation__SortingArm_H_
#define OpenKAI_src_Automation__SortingArm_H_

#include "../../../Detector/_DetectorBase.h"
#include "_Sequencer.h"
#include "_SortingCtrlServer.h"

#ifdef USE_OPENCV

namespace kai
{

class _SortingArm: public _ThreadBase
{
public:
	_SortingArm(void);
	virtual ~_SortingArm();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	void updateState(void);
	void updateArm(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SortingArm *) This)->update();
		return NULL;
	}

public:
	_SortingCtrlServer*	m_pCS;
	_Sequencer*		m_pSeq;
	vector<_ActuatorBase*> m_vAB;
	_Object		m_tO;

	int			m_nClass;
	vector<float> m_vDropPos;
	uint64_t	m_classFlag;
	vFloat2		m_rGripX;	//grip region X, target bb midX will be re-mapped into this region
	vFloat2		m_vRoiX;
	vFloat2		m_rGripY;	//grip region Y
	vFloat2		m_vRoiY;
	vFloat2		m_rGripZ;	//vertical
	int			m_iLastState;

	string		m_actuatorX;
	string		m_actuatorZ;
};

}
#endif
#endif
