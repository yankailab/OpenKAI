#ifndef OpenKAI_src_Autopilot_Rover__Rover_obstacle_H_
#define OpenKAI_src_Autopilot_Rover__Rover_obstacle_H_

#include "../../../Detector/_DetectorBase.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_obstacle: public _AutopilotBase
{
public:
	_Rover_obstacle();
	~_Rover_obstacle();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	void draw(void);

public:
	_DetectorBase* m_pDet;
	PIDctrl* m_pPID;
	ROVER_CTRL m_ctrl;

};

}
#endif
