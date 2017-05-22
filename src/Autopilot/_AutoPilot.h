#ifndef OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_
#define OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_

#include "../Base/common.h"
#include "../Automaton/_Automaton.h"
#include "ActionBase.h"
#include "Controller/RC/RC_base.h"
#include "Controller/APcopter/APcopter_base.h"
#include "Controller/APcopter/APcopter_DNNavoid.h"
#include "Controller/APcopter/APcopter_DNNnav.h"
#include "Controller/APcopter/APcopter_DNNodom.h"
#include "Controller/APcopter/APcopter_sensorAvoid.h"
#include "Controller/APcopter/APcopter_visualFollow.h"
#include "Controller/APcopter/APcopter_visualLanding.h"
#include "Controller/APcopter/APcopter_ZEDodom.h"
#include "Controller/AProver/AProver_base.h"
#include "Controller/AProver/AProver_follow.h"
#include "Controller/HM/HM_avoid.h"
#include "Controller/HM/HM_base.h"
#include "Controller/HM/HM_follow.h"
#include "Controller/HM/HM_grass.h"
#include "Controller/HM/HM_kickBack.h"
#include "Controller/HM/HM_rth.h"
#include "Controller/HM/HM_rth_approach.h"
#include "Controller/RC/RC_base.h"
#include "Controller/RC/RC_visualFollow.h"

#define ADD_ACTION(x) if(pAction->m_class==#x){*pA=new x();F_ERROR_F((*pA)->init(pAction));continue;}
#define N_ACTION 32

namespace kai
{

class _AutoPilot: public _ThreadBase
{
public:
	_AutoPilot();
	~_AutoPilot();

	bool init(void* pKiss);
	bool link(void);

	bool start(void);
	bool draw(void);

	void onMouse(MOUSE* pMouse);

public:
	_Automaton* m_pAM;

	int m_nAction;
	ActionBase* m_pAction[N_ACTION];

	//Thread
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AutoPilot *) This)->update();
		return NULL;
	}
};

}

#endif

