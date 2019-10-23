#ifndef OpenKAI_src_Autopilot_Rover__Rover_drive_H_
#define OpenKAI_src_Autopilot_Rover__Rover_drive_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_drive: public _AutopilotBase
{
public:
	_Rover_drive();
	~_Rover_drive();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updateDrive(void);
	static void* getUpdateThread(void* This)
	{
		((_Rover_base*) This)->update();
		return NULL;
	}

public:
	_Rover_CMD* m_pCMD;
	PIDctrl* m_pPID;
	ROVER_CTRL m_ctrl;

	float m_nSpeed;
	float m_dSpeed;

};

}
#endif
