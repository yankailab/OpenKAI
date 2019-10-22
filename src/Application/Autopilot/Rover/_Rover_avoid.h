#ifndef OpenKAI_src_Autopilot_Rover__Rover_avoid_H_
#define OpenKAI_src_Autopilot_Rover__Rover_avoid_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_avoid: public _AutopilotBase
{
public:
	_Rover_avoid();
	~_Rover_avoid();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	void draw(void);

public:
	_DetectorBase* m_pDet;
	PIDctrl* m_pPID;
	ROVER_CTRL m_ctrl;

	OBJECT m_obs;
	float m_dStop;

};

}
#endif
