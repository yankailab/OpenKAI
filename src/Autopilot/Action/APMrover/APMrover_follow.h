
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMROVER_FOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMROVER_FOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Stream/Frame.h"
#include "../../../Stream/Frame.h"
#include "../../../Navigation/_Universe.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Algorithm/Filter.h"
#include "../ActionBase.h"
#include "APMrover_base.h"

namespace kai
{

class APMrover_follow: public ActionBase
{
public:
	APMrover_follow();
	~APMrover_follow();

	bool init(Config* pConfig);
	bool link(void);
	void update(void);
	bool draw(Frame* pFrame, vInt4* pTextPos);

public:
	void followTarget(void);

private:
	APMrover_base* m_pAPM;
	_Universe*	m_pUniv;
	_Automaton* m_pAM;

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

