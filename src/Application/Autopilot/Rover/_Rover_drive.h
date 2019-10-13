#ifndef OpenKAI_src_Autopilot_Rover__Rover_drive_H_
#define OpenKAI_src_Autopilot_Rover__Rover_drive_H_

#include "../../../Detector/_DetectorBase.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_drive: public _AutopilotBase
{
public:
	_Rover_drive();
	~_Rover_drive();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updatePWM(void);

public:
	_AutopilotBase* m_pAB;
	_Rover_CMD* m_pCMD;

	ROVER_CTRL m_ctrl;

};

}
#endif
