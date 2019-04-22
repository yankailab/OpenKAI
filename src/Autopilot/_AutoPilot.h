#ifndef OpenKAI_src_Autopilot__AutoPilot_H_
#define OpenKAI_src_Autopilot__AutoPilot_H_

#include "../Base/common.h"
#include "ActionBase.h"
#include "Controller/APcopter/APcopter_avoid.h"
#include "Controller/APcopter/APcopter_base.h"
#include "Controller/APcopter/APcopter_depthVision.h"
#include "Controller/APcopter/APcopter_distLidar.h"
#include "Controller/APcopter/APcopter_DNNavoid.h"
#include "Controller/APcopter/APcopter_land.h"
#include "Controller/APcopter/APcopter_line.h"
#include "Controller/APcopter/APcopter_mode.h"
#include "Controller/APcopter/APcopter_takePhoto.h"
#include "Controller/APcopter/APcopter_posCtrl.h"
#include "Controller/APcopter/APcopter_slam.h"
#include "Controller/APcopter/APcopter_target.h"
#include "Controller/APcopter/APcopter_WP.h"
#include "Controller/APcopter/APcopter_RTH.h"
#include "Controller/AProver/AProver_base.h"
#include "Controller/Rover/Rover_base.h"
#include "Controller/Rover/Rover_move.h"
#include "Controller/Traffic/Traffic_alert.h"
#include "Controller/Traffic/Traffic_base.h"
#include "Controller/Traffic/Traffic_speed.h"
#include "Controller/Traffic/Traffic_videoOut.h"

#define ADD_ACTION(x) if(pAction->m_class==#x){A.m_pInst=new x();A.m_pKiss=pAction;}

namespace kai
{

struct ACTION_INST
{
	ActionBase* m_pInst;
	Kiss* m_pKiss;

	void init(void)
	{
		m_pInst = NULL;
		m_pKiss = NULL;
	}
};

class _AutoPilot: public _ThreadBase
{
public:
	_AutoPilot();
	~_AutoPilot();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);

public:
	vector<ACTION_INST> m_vAction;

	//Thread
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AutoPilot *) This)->update();
		return NULL;
	}
};

}
#endif
