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
#include "../Automaton/_Automaton.h"
#include "../Camera/Camera.h"
#include "../Interface/_Mavlink.h"
#include "../UI/UIMonitor.h"
#include "../Utility/util.h"
#include "../Vision/_Flow.h"
#include "../Detector/_Bullseye.h"
#include "../Detector/_AprilTags.h"
#include "../Tracker/_ROITracker.h"
#include "../AI/_SSD.h"
#include "../Detector/_Depth.h"
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
	_Automaton* m_pAM;
	_AutoPilot* m_pAP;
	_Universe* m_pUniverse;
	_Mavlink* m_pMavlink;
	_AprilTags* m_pAT;
	_SSD* m_pSSD;
	_Bullseye* m_pMD;
	_ROITracker* m_pROITracker;
	_Cascade* m_pCascade;
	_Depth* m_pDD;
	_FCN* m_pFCN;
	_Flow* m_pFlow;

	Frame* m_pFrame;

	bool start(Config* pConfig);
	void draw(void);
	void handleMouse(int event, int x, int y, int flags);
	void handleKey(int key);

};

}
#endif
