#ifndef OpenKAI_src_Autopilot_Rover__Rover_CMD_H_
#define OpenKAI_src_Autopilot_Rover__Rover_CMD_H_

#include "../../../Protocol/_ProtocolBase.h"

namespace kai
{

#define ROVERCMD_STATE 0
#define ROVERCMD_SPEED 1
#define ROVERCMD_PINOUT 2
#define ROVERCMD_DEBUG 3

#define FLT_SCALE 1000
#define FLT_SCALE_INV 0.001


enum ROVER_MODE
{
	rover_mode_unknown = 0,
	rover_stop = 1,
	rover_manual = 2,
	rover_auto1 = 3,
	rover_auto2 = 4,
};

enum ROVER_ACTION
{
	rover_action_unknown = 0,
	rover_action1 = 1,
	rover_action2 = 2,
	rover_action3 = 3,
};

const string c_roverModeName[] =
{
	"UNKNOWN",
	"STOP",
	"MANUAL",
	"AUTO",
	"FORWARD",
	"BACKWARD",
};

const string c_roverActionName[] =
{
	"UNKNOWN",
	"ACTION1",
	"ACTION2",
	"ACTION3",
};

class _Rover_CMD: public _ProtocolBase
{
public:
	_Rover_CMD();
	~_Rover_CMD();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

	void handleCMD(void);

	void sendState(int iState);
	void setSpeed(float nSpeed, float dSpeed);
	void pinOut(uint8_t iPin, uint8_t state);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Rover_CMD *) This)->update();
		return NULL;
	}

public:
	ROVER_MODE m_mode;
	ROVER_ACTION m_action;
	float m_nSpeed;
	float m_dSpeed;
};

}
#endif
