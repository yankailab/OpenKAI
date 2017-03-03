#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMPATHFINDER_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMPATHFINDER_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Vision/_VisionBase.h"
#include "../../../Navigation/_Obstacle.h"
#include "../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_pathFinder: public ActionBase
{
public:
	HM_pathFinder();
	~HM_pathFinder();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

	bool isPathFoundFront(void);
	bool isPathFoundSide(void);

private:
	HM_base* m_pHM;
	_Obstacle* m_pObs;

	int			m_rpmSteer;
	vDouble4 	m_searchArea;
	double		m_distDiff;
	uint64_t	m_turnStart;
	uint64_t	m_turnTimer;
	uint64_t	m_tLastTurn;
	uint64_t	m_turnInterval;
};

}

#endif

