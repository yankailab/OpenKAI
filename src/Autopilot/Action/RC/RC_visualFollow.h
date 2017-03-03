#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_RC_VISUALFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_RC_VISUALFOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Filter/FilterBase.h"
#include "../../../Script/Kiss.h"
#include "../../../Protocol/_RC.h"
#include "../../../Vision/Frame.h"
#include "../../../Tracker/_ROITracker.h"
#include "../../../Vision/_Flow.h"
#include "../../../UI/UI.h"
#include "../ActionBase.h"
#include "RC_base.h"

#ifndef USE_OPENCV4TEGRA

#define MODE_ASSIST 0
#define MODE_DRAWRECT 1

namespace kai
{

class RC_visualFollow: public ActionBase
{
public:
	RC_visualFollow();
	~RC_visualFollow();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);
	Rect2d getMouseROI(vInt4 mouseROI);

	void onMouse(MOUSE* pMouse);
	void onMouseAssist(MOUSE* pMouse, BUTTON* pBtn);
	void onMouseDrawRect(MOUSE* pMouse, BUTTON* pBtn);

private:
	_RC* m_pRC;
	RC_base* m_pRCconfig;

	//Control
	RC_CTRL m_roll;
	RC_CTRL m_pitch;
	RC_CTRL m_yaw;
	RC_CTRL m_alt;

	//Detectors
	_ROITracker* m_pROITracker;
	_Flow* m_pFlow;

	//UI
	bool m_bUI;
	UI* m_pUIassist;
	UI* m_pUIdrawRect;

	vInt4 m_ROI;
	bool m_bSelect;
	int m_ROImode;
	int m_ROIsize;
	int m_ROIsizeFrom;
	int m_ROIsizeTo;
	int m_ROIsizeStep;


};

}
#endif
#endif

