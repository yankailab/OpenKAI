#ifndef OpenKAI_src_Autopilot_Controller_APcopter_arucoLanding_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_arucoLanding_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

struct LANDING_TARGET_ARUCO
{
	int m_tag;
	double m_angle;

	void init(void)
	{
		m_tag = -1;
		m_angle = -1.0;
	}
};

class APcopter_arucoLanding: public ActionBase
{
public:
	APcopter_arucoLanding();
	~APcopter_arucoLanding();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);

	void navigation(void);

public:
	APcopter_base* m_pAP;
	_ObjectBase* m_pArUco;
	_DepthVisionBase* m_pDV;

	vDouble2 m_orientation;
	bool	 m_bLocked;
	OBJECT 	 m_oTarget;
	mavlink_landing_target_t m_D;
	vector<LANDING_TARGET_ARUCO> m_vTarget;

};

}
#endif
