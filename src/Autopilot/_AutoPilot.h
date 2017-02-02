#ifndef OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_
#define OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_

#include "../Base/common.h"
#include "../Automaton/_Automaton.h"
#include "Action/ActionBase.h"

#include "Action/HM/HM_base.h"
#include "Action/HM/HM_pathFinder.h"
#include "Action/HM/HM_grass.h"
#include "Action/HM/HM_rth.h"
#include "Action/HM/HM_avoid.h"
#include "Action/HM/HM_follow.h"

#include "Action/APMcopter/APMcopter_base.h"
#include "Action/APMcopter/APMcopter_landing.h"
#include "Action/APMcopter/APMcopter_guided.h"
#include "Action/APMcopter/APMcopter_avoid.h"
#include "Action/APMrover/APMrover_base.h"
#include "Action/APMrover/APMrover_follow.h"
#include "Action/RC/RC_base.h"
#include "Action/RC/RC_visualFollow.h"

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

