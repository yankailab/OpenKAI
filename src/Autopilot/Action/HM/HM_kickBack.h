#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMKICKBACK_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMKICKBACK_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Navigation/_Obstacle.h"
#include "../../../Navigation/_Path.h"
#include "../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_kickBack: public ActionBase
{
public:
	HM_kickBack();
	~HM_kickBack();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

	void reset(void);

private:
	HM_base* m_pHM;
	_Obstacle* m_pObs;
	_Path* m_pPath;

	int		m_rpmBack;
	double m_kickBackDist;

	UTM_POS	m_wpStation;
	UTM_POS	m_wpApproach;
	bool	m_bSetStation;

	double m_distM;
	vDouble4 m_obsBox;
	vInt2 m_posMin;

};

}

#endif

