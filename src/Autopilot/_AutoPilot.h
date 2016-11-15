#ifndef OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_
#define OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Automaton/_Automaton.h"
#include "Action/ActionBase.h"
#include "Action/APMcopter/APMcopter_landing.h"
#include "Action/HM/HM_follow.h"
#include "Action/RC/RC_visualFollow.h"

#define N_ACTION 32

namespace kai
{

class _AutoPilot: public _ThreadBase
{
public:
	_AutoPilot();
	~_AutoPilot();

	bool init(Config* pConfig);
	bool link(void);

	bool start(void);
	bool draw(Frame* pFrame, vInt4* pTextPos);

	void onMouse(MOUSE* pMouse);

public:
	_Automaton* m_pAM;

	int m_nAction;
	ActionBase* m_pAction[N_ACTION];
	ActionBase* m_pAA;

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

