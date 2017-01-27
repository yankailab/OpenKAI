#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMAVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMAVOID_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Stream/_StreamBase.h"
#include "../../../Navigation/_Obstacle.h"
#include "../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_avoid: public ActionBase
{
public:
	HM_avoid();
	~HM_avoid();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	HM_base* m_pHM;
	_StreamBase* m_pStream;
	_Obstacle* m_pObs;

	vDouble4 m_avoidArea;
	vInt2	m_posMin;
	double	m_alertDist;

	double	m_speedP;
	double	m_steerP;

	//operation
	double m_distM;

};

}

#endif

