/*
 * _PickingArm.h
 *
 *  Created on: June 30, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_RobotArm_Articulated__PickingArm_H_
#define OpenKAI_src_RobotArm_Articulated__PickingArm_H_

#include "../../../Universe/_Universe.h"
#include "../../../Actuator/_ActuatorBase.h"
#include "../../../Control/PIDctrl.h"

#ifdef USE_OPENCV

namespace kai
{

struct PICKING_CLASS
{
	int m_iClass;

	void init(void)
	{
		m_iClass = 0;
	}

};

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
	_Universe* m_pU;
	_Object		m_tO;

	vFloat3 m_vP;		//variable, screen coordinate of the object being followed
	vFloat3 m_vTargetP;	//constant, screen coordinate where the followed object should get to
	PIDctrl* m_pXpid;
	PIDctrl* m_pYpid;
	PIDctrl* m_pZpid;

	vector<PICKING_CLASS>	m_vClass;
};

}
#endif
#endif
