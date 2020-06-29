/*
 * _PickingArm.h
 *
 *  Created on: June 30, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_RobotArm_Articulated__PickingArm_H_
#define OpenKAI_src_RobotArm_Articulated__PickingArm_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Actuator/_ActuatorBase.h"

#ifdef USE_OPENCV

namespace kai
{

class _PickingArm: public _ThreadBase
{
public:
	_PickingArm(void);
	virtual ~_PickingArm();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	void updateArm(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PickingArm *) This)->update();
		return NULL;
	}

public:
	_ActuatorBase* m_pA;
	_DetectorBase* m_pD;
	_Object		m_tO;

	int			m_nClass;
	uint64_t	m_classFlag;

	vector<float> m_vDropPos;


};

}
#endif
#endif
