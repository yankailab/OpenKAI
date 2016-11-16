#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Stream/Frame.h"
#include "../../../Navigation/_Universe.h"
#include "../../../Algorithm/Filter.h"
#include "../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_follow: public ActionBase
{
public:
	HM_follow();
	~HM_follow();

	bool init(Config* pConfig);
	bool link(void);
	void update(void);
	bool draw(Frame* pFrame, vInt4* pTextPos);

private:
	_Universe*	m_pUniv;
	_Automaton* m_pAM;
	HM_base* m_pHM;

	//where target should be on cam coordinate
	double	m_destX;
	double	m_destY;
	double	m_destArea;

	double	m_speedP;
	double	m_steerP;

	OBJECT* m_pTarget;
	kai::Filter* m_pTargetX;
	kai::Filter* m_pTargetY;
	kai::Filter* m_pTargetArea;
	int		m_filterWindow;
	int		m_targetClass;

};

}

#endif

