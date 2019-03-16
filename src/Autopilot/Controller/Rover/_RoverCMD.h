#ifndef OpenKAI_src_Autopilot_Rover__RoverCMD_H_
#define OpenKAI_src_Autopilot_Rover__RoverCMD_H_

#include "../../../Protocol/_Arduino.h"

namespace kai
{

enum ROVER_MODE
{
	rover_idle = 0,
	rover_manual = 1,
	rover_auto = 2,
	rover_forward = 3,
	rover_backward = 4,
};

class _RoverCMD: public _Arduino
{
public:
	_RoverCMD();
	~_RoverCMD();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);

	void handleCMD(void);

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

};

}
#endif
