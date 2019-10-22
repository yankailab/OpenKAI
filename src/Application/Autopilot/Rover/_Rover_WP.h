#ifndef OpenKAI_src_Autopilot_Rover__Rover_WP_H_
#define OpenKAI_src_Autopilot_Rover__Rover_WP_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_WP: public _AutopilotBase
{
public:
	_Rover_WP();
	~_Rover_WP();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	void draw(void);

public:
	_Mavlink* m_pMavlink;
	_Rover_CMD* m_pCMD;
	PIDctrl* m_pPIDhdg;

	ROVER_CTRL m_ctrl;

};

}
#endif
