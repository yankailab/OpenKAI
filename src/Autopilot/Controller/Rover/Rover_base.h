#ifndef OpenKAI_src_Autopilot_Controller_Rover_Rover_base_H_
#define OpenKAI_src_Autopilot_Controller_Rover_Rover_base_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Arduino.h"
#include "../../ActionBase.h"

namespace kai
{

enum ROVER_MODE
{
	rover_unknown = -1,
	rover_idle = 0,
	rover_manual = 1,
	rover_forward = 2,
	rover_backward = 3,
	rover_auto = 4,
};

class Rover_base: public ActionBase
{
public:
	Rover_base();
	~Rover_base();

	bool init(void* pKiss);
	int	 check(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);

	void cmd(void);

public:
	_Arduino* m_pA;

	float m_hdg;	//-1.0 to 1.0
	float m_speed;	//-1.0 to 1.0

	ROVER_MODE m_rMode;
};

}
#endif
