#ifndef OpenKAI_src_Autopilot_Rover__Rover_CMD_H_
#define OpenKAI_src_Autopilot_Rover__Rover_CMD_H_

#include "../../../Protocol/_ProtocolBase.h"

namespace kai
{

#define ROVERCMD_STATE 0
#define ROVERCMD_PWM 1
#define ROVERCMD_PINOUT 2

enum ROVER_MODE
{
	rover_unknown = -1,
	rover_idle = 0,
	rover_manual = 1,
	rover_auto = 2,
	rover_forward = 3,
	rover_backward = 4,
};

const string c_roverModeName[] =
{
	"IDLE",
	"MANUAL",
	"AUTO",
	"FORWARD",
	"BACKWARD",
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
	void setPWM(uint16_t* pPWM, int nCh);
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
	uint8_t	m_nPwmIn;
	uint16_t* m_pPwmIn;

};

}
#endif
