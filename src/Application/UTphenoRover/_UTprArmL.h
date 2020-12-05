#ifndef OpenKAI_src_Application_UTphenoRover__UTprArmL_H_
#define OpenKAI_src_Application_UTphenoRover__UTprArmL_H_

#include "../../Universe/_Universe.h"
#include "../../Control/PIDctrl.h"
#include "../../Actuator/_ActuatorBase.h"
#include "_AProver_UTfollowTag.h"

namespace kai
{

class _UTprArmL: public _MissionBase
{
public:
	_UTprArmL(void);
	virtual ~_UTprArmL();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

	void stop(void);

private:
	bool recover(void);
	bool follow(void);
	bool ascend(void);

	_Object* findTarget(void);
	void updateArm(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_UTprArmL *) This)->update();
		return NULL;
	}

public:
	_ActuatorBase* m_pAx;
	_ActuatorBase* m_pAy;
	_ActuatorBase* m_pAz;
	_Universe* m_pU;
    PIDctrl* m_pXpid;
	PIDctrl* m_pYpid;

	vFloat3 m_vP;		//x,y:variable, screen coordinate of the object being followed, z:variable in mm unit
	vFloat3 m_vPtarget;	//x,y:constant, screen coordinate where the followed object should get to, z:variable in mm unit
	vFloat3 m_vPrecover;//arm stock pos
	float m_zSpeed;
	float m_zrK;
	vFloat2 m_vZgoal;

	UTPR_MISSIOIN m_iMission;
};

}
#endif
