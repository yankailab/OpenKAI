#ifndef OpenKAI_src_Autopilot_Rover__Rover_follow_H_
#define OpenKAI_src_Autopilot_Rover__Rover_follow_H_

#include "../../../Detector/_DetectorBase.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_follow: public _AutopilotBase
{
public:
	_Rover_follow();
	~_Rover_follow();

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
