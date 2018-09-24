#ifndef OpenKAI_src_Autopilot__AutoPilot_H_
#define OpenKAI_src_Autopilot__AutoPilot_H_

#include "../Base/common.h"
#include "../Automaton/_Automaton.h"
#include "ActionBase.h"
#include "Controller/APcopter/APcopter_arucoLanding.h"
#include "Controller/APcopter/APcopter_arucoFollow.h"
#include "Controller/APcopter/APcopter_base.h"
#include "Controller/APcopter/APcopter_depthVision.h"
#include "Controller/APcopter/APcopter_distLidar.h"
#include "Controller/APcopter/APcopter_DNNavoid.h"
#include "Controller/APcopter/APcopter_DNNlanding.h"
#include "Controller/APcopter/APcopter_DNNnav.h"
#include "Controller/APcopter/APcopter_edgeHold.h"
#include "Controller/APcopter/APcopter_followPos.h"
#include "Controller/APcopter/APcopter_followRC.h"
#include "Controller/APcopter/APcopter_posCtrlRC.h"
#include "Controller/APcopter/APcopter_mission.h"
#include "Controller/AProver/AProver_base.h"
#include "Controller/HM/HM_avoid.h"
#include "Controller/HM/HM_base.h"
#include "Controller/HM/HM_follow.h"
#include "Controller/HM/HM_grass.h"
#include "Controller/HM/HM_kickBack.h"
#include "Controller/HM/HM_marker.h"
#include "Controller/HM/HM_rth.h"
#include "Controller/HM/HM_rth_approach.h"
#include "Controller/Traffic/Traffic_alert.h"
#include "Controller/Traffic/Traffic_base.h"
#include "Controller/Traffic/Traffic_speed.h"
#include "Controller/VEK/VEK_avoid.h"
#include "Controller/VEK/VEK_base.h"
#include "Controller/VEK/VEK_follow.h"

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
	bool cli(int& iY);

public:
	_Automaton* m_pAM;
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
