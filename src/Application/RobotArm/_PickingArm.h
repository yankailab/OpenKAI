/*
 * _PickingArm.h
 *
 *  Created on: June 30, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_RobotArm__PickingArm_H_
#define OpenKAI_src_RobotArm__PickingArm_H_

#include "../Mission/_MissionBase.h"
#include "../../Universe/_Universe.h"
#include "../../Control/PIDctrl.h"
#include "../../Actuator/_ActuatorBase.h"
#include "../../Actuator/_StepperGripper.h"
#include "../../Sensor/_DistSensorBase.h"

#ifdef USE_OPENCV

namespace kai
{

struct PICKINGARM_CLASS
{
	int m_iClass;

	void init(void)
	{
		m_iClass = 0;
	}

};

struct PICKINGARM_MISSION
{
	int8_t EXTERNAL;
	int8_t RECOVER;
	int8_t FOLLOW;
	int8_t ASCEND;
	int8_t DELIVER;
	int8_t DESCEND;
	int8_t DROP;

	bool bValid(void)
	{
		IF_F(EXTERNAL < 0);
		IF_F(RECOVER < 0);
		IF_F(FOLLOW < 0);
		IF_F(ASCEND < 0);
		IF_F(DELIVER < 0);
		IF_F(DESCEND < 0);
		IF_F(DROP < 0);

		return true;
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

	void move(vFloat3& vM);
	void rotate(vFloat3& vR);
	void stop(void);
	void grip(bool bOpen);

private:
	void speed(const vFloat3& vS);
	void external(void);
	bool recover(void);
	bool follow(void);
	bool ascend(void);
	bool deliver(void);
	bool descend(void);
	bool drop(void);

	_Object* findTarget(void);
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

	float	m_baseAngle;
	vFloat3 m_vP;		//x,y:variable, screen coordinate of the object being followed, z:variable in mm unit
	vFloat3 m_vPtarget;	//x,y:constant, screen coordinate where the followed object should get to, z:variable in mm unit
	PIDctrl* m_pXpid;
	PIDctrl* m_pYpid;
	PIDctrl* m_pZpid;
	vFloat4	m_vZrange;
	float m_zSpeed;
	float m_zrK;

	vFloat3 m_vPrecover;
	vFloat3 m_vPdeliver;
	vFloat3 m_vPdescend;

	vFloat3 m_vS;
	vFloat3 m_vR;

	vector<PICKINGARM_CLASS>	m_vClass;

	PICKINGARM_MISSION m_iMission;
};

}
#endif
#endif
