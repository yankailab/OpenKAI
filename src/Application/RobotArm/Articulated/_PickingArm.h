/*
 * _PickingArm.h
 *
 *  Created on: June 30, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_RobotArm_Articulated__PickingArm_H_
#define OpenKAI_src_RobotArm_Articulated__PickingArm_H_

#include "../../Mission/_MissionBase.h"
#include "../../../Universe/_Universe.h"
#include "../../../Control/PIDctrl.h"
#include "../../../Actuator/_ActuatorBase.h"
#include "../../../Actuator/_StepperGripper.h"
#include "../../../Sensor/_DistSensorBase.h"

#ifdef USE_OPENCV

namespace kai
{

enum PICKINGARM_MODE
{
	paMode_unknown = 0,
	paMode_external = 1,
	paMode_auto = 2,
};

enum PICKINGARM_STATE
{
	pa_standby,
	pa_search,
	pa_catch,
	pa_recover,
};

struct PICKINGARM_CLASS
{
	int m_iClass;

	void init(void)
	{
		m_iClass = 0;
	}

};

class _PickingArm: public _MissionBase
{
public:
	_PickingArm(void);
	virtual ~_PickingArm();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

	void setMode(PICKINGARM_MODE m);
	void move(vFloat3& vM);
	void rotate(vFloat3& vR);
	void grip(bool bOpen);

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
	_StepperGripper* m_pG;
	_DistSensorBase* m_pD;
	_Universe* m_pU;
	PICKINGARM_MODE m_mode;
	PICKINGARM_STATE m_state;

	float	m_baseAngle;
	vFloat3 m_vP;		//variable, screen coordinate of the object being followed
	vFloat3 m_vTargetP;	//constant, screen coordinate where the followed object should get to
	PIDctrl* m_pXpid;
	PIDctrl* m_pYpid;
	PIDctrl* m_pZpid;

	vFloat3 m_vM;
	vFloat3 m_vR;

	vector<PICKINGARM_CLASS>	m_vClass;
};

}
#endif
#endif
