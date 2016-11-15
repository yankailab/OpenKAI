#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Stream/Frame.h"
#include "../../../Navigation/_Universe.h"
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
	void updateMotion(void);

	_Universe*	m_pUniv;
	_Automaton* m_pAM;
	HM_base* m_pHM;

};

}

#endif

