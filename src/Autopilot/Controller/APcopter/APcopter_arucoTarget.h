#ifndef OpenKAI_src_Autopilot_Controller_APcopter_arucoTarget_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_arucoTarget_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

struct ARUCO_TARGET
{
	int m_tag;
	double m_angle;

	void init(void)
	{
		m_tag = -1;
		m_angle = -1.0;
	}
};

class APcopter_arucoTarget: public ActionBase
{
public:
	APcopter_arucoTarget();
	~APcopter_arucoTarget();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

	void updateGimbal(void);

public:
	APcopter_base* m_pAP;
	_ObjectBase* m_pArUco;
	_DepthVisionBase* m_pDV;

	vDouble2 m_orientation;
	bool	 m_bLocked;
	OBJECT 	 m_oTarget;
	mavlink_landing_target_t m_D;
	vector<ARUCO_TARGET> m_vTarget;

	vDouble3 m_vGimbal;
	mavlink_mount_control_t m_gimbalControl;
	mavlink_mount_configure_t m_gimbalConfig;

};

}
#endif
