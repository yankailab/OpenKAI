/*
 * demo.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_VISUALFOLLOW_H_
#define SRC_VISUALFOLLOW_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "../Camera/Camera.h"
#include "../Interface/_Mavlink.h"
#include "../Interface/_RC.h"
#include "../Utility/util.h"
#include "../Vision/_Flow.h"
#include "../Tracker/_ROITracker.h"
#include "../Application/AppBase.h"
#include "../Autopilot/_AutoPilot.h"

#define APP_NAME "VISUAL_FOLLOW"

#define MODE_ASSIST 0
#define MODE_RECTDRAW 1

using namespace kai;

namespace kai
{

class VisualFollow: public AppBase
{
public:
	VisualFollow();
	~VisualFollow();

	_Stream* m_pCamFront;
	_AutoPilot* m_pAP;
	_Mavlink* m_pMavlink;
	_RC* m_pVlink;
	_Flow* m_pDF;
	_ROITracker* m_pROITracker;

	Frame* m_pFrame;

	iVector4 m_ROI;
	bool	 m_bSelect;
	int		 m_minROI;
	int		 m_ROImode;
	int		 m_ROIsize;
	int 	 m_ROIsizeFrom;
	int		 m_ROIsizeTo;

	//UI
	int		m_btnSize;
	int		m_btnROIClear;
	int		m_btnROIBig;
	int		m_btnROISmall;
	int		m_btnMode;

	Rect2d getROI(iVector4 mouseROI);

	bool start(JSON* pJson);
	void draw(void);
	void handleMouse(int event, int x, int y, int flags);
	void handleKey(int key);

};

}
#endif
