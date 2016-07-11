/*
 * demo.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_DRONEHUNTER_H_
#define SRC_DRONEHUNTER_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "../AI/_SegNet.h"
#include "../Autopilot/_AutoPilot.h"
#include "../Camera/CamInput.h"
#include "../Interface/_MavlinkInterface.h"
#include "../Interface/_VehicleInterface.h"
#include "../UI/UIMonitor.h"
#include "../Utility/util.h"
#include "../Image/_DenseFlow.h"
#include "../Image/_3DFlow.h"
#include "../Tracker/_ROITracker.h"

#define APP_NAME "VISUAL_FOLLOW"

#define MODE_ASSIST 0
#define MODE_RECTDRAW 1

using namespace kai;

namespace kai
{

class VisualFollow
{
public:
	VisualFollow();
	~VisualFollow();

	int m_key;
	bool m_bRun;

	_CamStream* m_pCamFront;
	_AutoPilot* m_pAP;
	_MavlinkInterface* m_pMavlink;
	_VehicleInterface* m_pVlink;
	_CascadeDetector* m_pCascade;
	_DenseFlow* m_pDF;
	_3DFlow* m_pDFDepth;
	_ROITracker* m_pROITracker;

	CamFrame* m_pShow;
	CamFrame* m_pMat;
	CamFrame* m_pMat2;
	UIMonitor* m_pUIMonitor;

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
	void showScreen(void);

	void handleMouse(int event, int x, int y, int flags);
	void showInfo(UMat* pDisplayMat);
	void handleKey(int key);

};

}
#endif
