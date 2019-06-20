#ifndef OpenKAI_src_Autopilot__AutoPilot_H_
#define OpenKAI_src_Autopilot__AutoPilot_H_

#include "../../Base/common.h"
#include "ActionBase.h"
#include "APcopter/APcopter_avoid.h"
#include "APcopter/APcopter_base.h"
#include "APcopter/APcopter_depthVision.h"
#include "APcopter/APcopter_distLidar.h"
#include "APcopter/APcopter_DNNavoid.h"
#include "APcopter/APcopter_land.h"
#include "APcopter/APcopter_line.h"
#include "APcopter/APcopter_mode.h"
#include "APcopter/APcopter_takePhoto.h"
#include "APcopter/APcopter_posCtrl.h"
#include "APcopter/APcopter_slam.h"
#include "APcopter/APcopter_target.h"
#include "APcopter/APcopter_WP.h"
#include "APcopter/APcopter_RTH.h"
#include "AProver/AProver_base.h"
#include "Rover/Rover_base.h"
#include "Rover/Rover_move.h"

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

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AutoPilot *) This)->update();
		return NULL;
	}
};

}
#endif
