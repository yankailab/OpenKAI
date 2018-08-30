#ifndef OpenKAI_src_Autopilot_Controller_VEK_VEK_follow_H_
#define OpenKAI_src_Autopilot_Controller_VEK_VEK_follow_H_

#include "../../../Base/common.h"
#include "../../../Detector/_ClusterNet.h"
//#include "../../../Sensor/_ZEDdistance.h"
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
	void update(void);
	bool draw(void);

private:
	VEK_base* m_pVEK;
//	_ZEDdistance* m_pObs;
#ifdef USE_TENSORRT
	_ClusterNet* m_pCN;
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
