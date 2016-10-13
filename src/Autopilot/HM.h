#ifndef OPENKAI_SRC_AUTOPILOT_HM_H_
#define OPENKAI_SRC_AUTOPILOT_HM_H_

#include "../Base/common.h"
#include "ActionBase.h"
#include "../UI/UI.h"

namespace kai
{

class HM: public ActionBase
{
public:
	HM();
	~HM();

	bool init(Config* pConfig, AUTOPILOT_CONTROL* pAC);
	void update(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);

private:

	//Detectors
	_ROITracker* m_pROITracker;

};

}

#endif

