#ifndef OpenKAI_src_Autopilot_Controller_HM_avoid_H_
#define OpenKAI_src_Autopilot_Controller_HM_avoid_H_

#include "../../../Base/common.h"
#include "../../../Mission/_MissionControl.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_avoid: public ActionBase
{
public:
	HM_avoid();
	~HM_avoid();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);

private:
	HM_base* m_pHM;
	_DepthVisionBase* m_pDV;

	int		m_rpmSteer;
	double m_distM;
	vDouble4 m_dROIf;
	vDouble4 m_dROIl;
	vDouble4 m_dROIr;
	vInt2 m_posMin;
	double m_alertDist;

};

}

#endif

