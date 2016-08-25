/*
 * demo.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_NAVIGATOR_H_
#define SRC_NAVIGATOR_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "../AI/_FCN.h"
#include "../Autopilot/_AutoPilot.h"
#include "../Camera/Camera.h"
#include "../Interface/_Mavlink.h"
#include "../UI/UIMonitor.h"
#include "../Utility/util.h"
#include "../Vision/_Flow.h"
#include "../Detector/_DepthDetector.h"
#include "../Detector/_MarkerDetector.h"
#include "../Tracker/_ROITracker.h"
#include "../AI/_SSD.h"
#include "../Navigation/_Universe.h"
#include "AppBase.h"

#define APP_NAME "NAVIGATOR"

using namespace kai;

namespace kai
{

class Navigator: public AppBase
{
public:
	Navigator();
	~Navigator();

	_Stream* m_pCamFront;
	_AutoPilot* m_pAP;
	_Mavlink* m_pMavlink;
	_CascadeDetector* m_pCascade;
	_Flow* m_pFlow;
	_ROITracker* m_pROITracker;
	_DepthDetector* m_pDD;
	_MarkerDetector* m_pMD;
	_Universe* m_pUniverse;
	_FCN* m_pFCN;
	_SSD* m_pSSD;

	Frame* m_pFrame;

	bool start(JSON* pJson);
	void draw(void);
	void handleMouse(int event, int x, int y, int flags);
	void handleKey(int key);

};

}
#endif
