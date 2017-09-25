#ifndef OpenKAI_src_Autopilot_Controller_VEK_VEK_follow_H_
#define OpenKAI_src_Autopilot_Controller_VEK_VEK_follow_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Sensor/_ZEDobstacle.h"
#include "../../ActionBase.h"
#include "VEK_base.h"

namespace kai
{

class VEK_follow: public ActionBase
{
public:
	VEK_follow();
	~VEK_follow();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	VEK_base* m_pVEK;
	_ZEDobstacle* m_pObs;
#ifdef USE_TENSORRT
	_DetectorBase* m_pDB;
#endif

	double	m_vSteer;
	double	m_distMin;
	double	m_distMax;
	double	m_targetX;
	double	m_rTargetX;
	OBJECT* m_pTarget;
	int		m_iTargetClass;
};

}

#endif

