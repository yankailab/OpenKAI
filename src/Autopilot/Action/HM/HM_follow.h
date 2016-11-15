#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMFOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Stream/Frame.h"
#include "../ActionBase.h"
#include "HM_base.h"

#ifdef USE_TENSORRT
#include "../AI/_DetectNet.h"
#endif
#ifdef USE_SSD
#include "../AI/_SSD.h"
#endif

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

	//Detectors
#ifdef USE_TENSORRT
	_DetectNet* m_pDN;
#endif
#ifdef USE_SSD
	_SSD* m_pSSD;
#endif

	HM_base* m_pHM;
	_Automaton* m_pAM;



};

}

#endif

