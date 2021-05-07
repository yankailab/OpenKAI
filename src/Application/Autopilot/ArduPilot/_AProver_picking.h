#ifndef OpenKAI_src_Autopilot_AP__AProver_picking_H_
#define OpenKAI_src_Autopilot_AP__AProver_picking_H_

#include "../../RobotArm/_PickingArm.h"
#include "../../../Utility/RC.h"
#include "_AP_base.h"
#include "_AProver_drive.h"

#ifdef USE_OPENCV

namespace kai
{

struct AP_PICKING_MISSION
{
	int8_t STANDBY;
	int8_t MANUAL;
	int8_t AUTOPICK;
	int8_t AUTO;

	bool bValid(void)
	{
		IF_F(STANDBY < 0);
		IF_F(MANUAL < 0);
		IF_F(AUTOPICK < 0);
		IF_F(AUTO < 0);

		return true;
	}
};

class _AProver_picking: public _StateBase
{
public:
	_AProver_picking();
	~_AProver_picking();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void console(void* pConsole);

protected:
	bool updateDrive(void);
	bool updatePicking(void);
	static void* getUpdate(void* This)
	{
		((_AProver_picking *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;
	_AProver_drive* m_pDrive;

	_PickingArm* m_pArm;
	_StateControl* m_pArmMC;

	RC_CHANNEL m_rcMode;
	vector<RC_CHANNEL>	m_vRC; //x,y,z,yaw,gripper

	AP_PICKING_MISSION m_iState;

};

}
#endif
#endif
