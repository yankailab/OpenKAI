#ifndef OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_
#define OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Automaton/_Automaton.h"
#include "ActionBase.h"
#include "VisualFollow.h"
#include "Landing.h"


//FALCON COMMANDS
#define CMD_RC_UPDATE 0
#define CMD_OPERATE_MODE 1

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
	bool draw(Frame* pFrame, iVec4* pTextPos);
	void sendHeartbeat(void);

	void onMouse(MOUSE* pMouse);


public:
	_Automaton* m_pAM;

	int m_nAction;
	ActionBase* m_pAction[N_ACTION];
	ActionBase* m_pAA;
	AUTOPILOT_CONTROL m_ctrl;

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

