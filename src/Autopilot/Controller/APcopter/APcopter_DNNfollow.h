#ifndef OpenKAI_src_Autopilot_Controller_APcopter_DNNfollow_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_DNNfollow_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../../../Navigation/GPS.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrlRC.h"

namespace kai
{

class APcopter_DNNfollow: public ActionBase
{
public:
	APcopter_DNNfollow();
	~APcopter_DNNfollow();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

	OBJECT* newFound(void);

public:
	APcopter_base* m_pAP;
	APcopter_posCtrlRC* m_pPC;
	_ObjectBase* m_pDet;
	uint64_t m_tStampDet;
	_TrackerBase* m_pTracker;
	GPS m_GPS;

	bool	 m_bUseTracker;
	bool	 m_bTarget;
	int		 m_iClass;

	vDouble4 m_vTarget;
	vDouble4 m_vPos;

	vDouble3 m_vGimbal;
};

}
#endif

/*
MAV_CMD_DO_MOUNT_CONFIGURE
mavutil.mavlink.MAV_MOUNT_MODE_MAVLINK_TARGETING,
#mount_mode
1, # stabilize roll
1, # stabilize pitch
1, # stabilize yaw

MAV_CMD_DO_MOUNT_CONTROL
pitch 100, # pitch is in centidegrees
roll 100, # roll
yaw * 100, # yaw is in centidegrees
0) # save position
 */
