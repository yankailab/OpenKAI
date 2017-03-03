#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMKICKBACK_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMKICKBACK_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
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

public:
	HM_base* m_pHM;
	_Path* m_pPath;

	int		m_rpmBack;
	double	m_kickBackDist;
	double	m_dist;

	UTM_POS	m_wpStation;
	UTM_POS	m_wpApproach;
	bool	m_bSetStation;

};

}

#endif

