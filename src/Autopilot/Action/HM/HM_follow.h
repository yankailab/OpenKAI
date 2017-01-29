#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../AI/_DetectNet.h"
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
	HM_base* m_pHM;

	double	m_speedP;
	double	m_steerP;

	vDouble2	m_targetPos;
	double		m_minProb;


};

}

#endif

