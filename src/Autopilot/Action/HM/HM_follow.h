#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../AI/_DetectNet.h"
#include "../../../Navigation/_Obstacle.h"
#include "../ActionBase.h"
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
	_DetectNet*	m_pDN;
	HM_base*	m_pHM;
	_Obstacle* 	m_pObs;

	double		m_speedP;
	double		m_steerP;
	double		m_distMin;
	double		m_distMax;
	double		m_targetX;
	double		m_minProb;
	uint64_t 	m_objLifetime;
	OBJECT* 	m_pTarget;
};

}

#endif

