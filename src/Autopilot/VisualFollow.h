#ifndef OPENKAI_SRC_AUTOPILOT_VISUALFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_VISUALFOLLOW_H_

#include "../Base/common.h"
#include "ActionBase.h"
#include "../UI/UI.h"

#define MODE_ASSIST 0
#define MODE_DRAWRECT 1

namespace kai
{

class VisualFollow: public ActionBase
{
public:
	VisualFollow();
	~VisualFollow();

	bool init(Config* pConfig, AUTOPILOT_CONTROL* pAC);
	void update(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);
	Rect2d getMouseROI(iVec4 mouseROI);

	void onMouse(MOUSE* pMouse);
	void onMouseAssist(MOUSE* pMouse, BUTTON* pBtn);
	void onMouseDrawRect(MOUSE* pMouse, BUTTON* pBtn);

private:
	//Control
	CONTROL_CHANNEL m_roll;
	CONTROL_CHANNEL m_pitch;
	CONTROL_CHANNEL m_yaw;
	CONTROL_CHANNEL m_alt;

	//Detectors
	_Cascade* m_pFD;
	_ROITracker* m_pROITracker;
	_Flow* m_pFlow;

	//UI
	bool m_bUI;
	UI* m_pUIassist;
	UI* m_pUIdrawRect;

	iVec4 m_ROI;
	bool m_bSelect;
	int m_ROImode;
	int m_ROIsize;
	int m_ROIsizeFrom;
	int m_ROIsizeTo;
	int m_ROIsizeStep;


};

}

#endif

