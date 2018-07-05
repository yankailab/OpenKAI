
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_arucoLanding_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_arucoLanding_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

struct LANDING_TARGET_ARUCO
{
	int m_code;
	double m_angle;
	int m_iPriority;

	void init(void)
	{
		m_code = -1;
		m_angle = -1.0;
		m_iPriority = 0;
	}
};

class APcopter_arucoLanding: public ActionBase
{
public:
	APcopter_arucoLanding();
	~APcopter_arucoLanding();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);
	bool cli(int& iY);

	void navigation(void);

public:
	APcopter_base* m_pAP;
	_DetectorBase* m_pDet;

	OBJECT 	 m_target;
	vDouble2 m_orientation;
	vDouble2 m_tAngle;
	bool	 m_bLocked;
	vector<LANDING_TARGET_ARUCO> m_vTarget;

};

}
#endif
