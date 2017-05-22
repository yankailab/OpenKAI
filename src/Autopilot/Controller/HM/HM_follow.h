#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Sensor/_Obstacle.h"
#include "../../../DNN/Detector/_MatrixNet.h"
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
	_MatrixNet*	m_pMN;
	HM_base*	m_pHM;
	_Obstacle* 	m_pObs;

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

