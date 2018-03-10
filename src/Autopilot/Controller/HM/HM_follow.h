#ifndef OpenKAI_src_Autopilot_Controller_HM_follow_H_
#define OpenKAI_src_Autopilot_Controller_HM_follow_H_

#include "../../../Base/common.h"
#include "../../../Detector/_ClusterNet.h"
#include "../../../Sensor/_ZEDdistance.h"
#include "../../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_follow: public ActionBase
{
public:
	HM_follow();
	~HM_follow();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	_ClusterNet*	m_pMN;
	HM_base*	m_pHM;
	_ZEDdistance* 	m_pObs;

	double		m_rpmT;
	double		m_rpmSteer;
	double		m_distMin;
	double		m_distMax;
	double		m_targetX;
	double		m_rTargetX;
	OBJECT* 	m_pTarget;
	int			m_iTargetClass;
	string		m_targetName;
};

}


#endif

