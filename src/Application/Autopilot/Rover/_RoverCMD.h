#ifndef OpenKAI_src_Autopilot_Rover__RoverCMD_H_
#define OpenKAI_src_Autopilot_Rover__RoverCMD_H_

#include "../../../Protocol/_ProtocolBase.h"

namespace kai
{

#define ROVERCMD_STATE 0
#define ROVERCMD_PWM 1
#define ROVERCMD_PINOUT 2

enum ROVER_MODE
{
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

class _RoverCMD: public _ProtocolBase
{
public:
	_RoverCMD();
	~_RoverCMD();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);

	void handleCMD(void);

	void sendState(int iState);
	void setPWM(int nChan, uint16_t* pChan);
	void pinOut(uint8_t iPin, uint8_t state);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RoverCMD *) This)->update();
		return NULL;
	}

public:
	uint8_t m_mode;
	uint16_t m_pwmModeIn;
	uint16_t m_pwmLin;
	uint16_t m_pwmRin;

	uint16_t m_pwmLmot;
	uint16_t m_pwmRmot;
};

}
#endif
