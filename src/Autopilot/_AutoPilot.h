
#ifndef OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_
#define OPENKAI_SRC_AUTOPILOT__AUTOPILOT_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Interface/_Mavlink.h"
#include "../Interface/_RC.h"
#include "../Automaton/_Automaton.h"
#include "ActionBase.h"
#include "commonAutopilot.h"


#define NUM_RC_CHANNEL 8

//FALCON COMMANDS
#define CMD_RC_UPDATE 0
#define CMD_OPERATE_MODE 1

namespace kai
{

class _AutoPilot: public _ThreadBase
{
public:
	_AutoPilot();
	~_AutoPilot();

	bool init(Config* pConfig, string name);
	bool start(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);
	void sendHeartbeat(void);

//	int* getPWMOutput(void);
//	void resetAllControl(void);

public:
	_Automaton* m_pAM;

	//Interface
	_RC* m_pVI;
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	//Control
	CONTROL_CHANNEL m_roll;
	CONTROL_CHANNEL m_pitch;
	CONTROL_CHANNEL m_yaw;
	CONTROL_CHANNEL m_alt;
	int m_RC[NUM_RC_CHANNEL];

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

