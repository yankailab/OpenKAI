#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_VEK_follow_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_VEK_follow_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_Obstacle.h"
#include "../../../DNN/Detector/_DetectNet.h"
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
	_Obstacle* m_pObs;
#ifdef USE_TENSORRT
	_DetectNet* m_pDN;
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

