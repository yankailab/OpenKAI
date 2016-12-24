#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMAVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMAVOID_H_

#include "../../../Base/common.h"
#include "../../../Algorithm/Filter.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Stream/_ZED.h"
#include "../../../Object/Object.h"
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

	double	m_speedP;
	double	m_steerP;
	int	m_pwmL;
	int m_pwmR;

	vDouble4 m_avoidRegion;
	double	m_avoidMinSize;
	double	m_alertDist;

	//normalized relevant distance: 0.0 ~ 1.0
	Filter*	m_pFdist;

	Object* m_pObj;

};

}

#endif

