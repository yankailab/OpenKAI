/*
 * demo.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_SEGNETDEMO_H_
#define SRC_SEGNETDEMO_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "../AI/_SegNet.h"
#include "../Autopilot/_AutoPilot.h"
#include "../Camera/CamInput.h"
#include "../Interface/_MavlinkInterface.h"
#include "../UI/UIMonitor.h"
#include "../Utility/util.h"

#define APP_NAME "SEGNET_INTERFACE_DEMO"

using namespace kai;

namespace kai
{

class SegNetDemo
{
public:
	SegNetDemo();
	~SegNetDemo();

	int m_key;
	bool m_bRun;
	int m_showMode;

	_CamStream* m_pCamFront;
	_AutoPilot* m_pAP;
	_MavlinkInterface* m_pMavlink;
	_SegNet* m_pSegNet;

	CamFrame* m_pShow;
	CamFrame* m_pMat;
	CamFrame* m_pMat2;
	UIMonitor* m_pUIMonitor;

	bool start(JSON* pJson);

	void showScreen(void);

	void handleMouse(int event, int x, int y, int flags);
	void showInfo(UMat* pDisplayMat);
	void handleKey(int key);

};

}
#endif
