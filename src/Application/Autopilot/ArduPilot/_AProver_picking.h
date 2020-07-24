#ifndef OpenKAI_src_Autopilot_AP__AProver_picking_H_
#define OpenKAI_src_Autopilot_AP__AProver_picking_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "../../RobotArm/Articulated/_PickingArm.h"
#include "../../../Actuator/_StepperGripper.h"
#include "../../../Utility/RC.h"
#include "_AP_base.h"
#include "_AProver_drive.h"

#ifdef USE_OPENCV

namespace kai
{

class _AProver_picking: public _MissionBase
{
public:
	_AProver_picking();
	~_AProver_picking();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);

protected:
	bool updateDrive(void);
	bool updatePicking(void);
	static void* getUpdateThread(void* This)
	{
		((_AProver_picking *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;
	_AProver_drive* m_pDrive;
	RC_CHANNEL m_rcMode;

	_PickingArm* m_pArm;
	_StepperGripper* m_pGripper;
	vector<RC_CHANNEL>	m_vRC; //x,y,z,yaw,gripper

};

}
#endif
#endif
