/*
 * demo.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_EXTCAMCONTROL_H_
#define SRC_EXTCAMCONTROL_H_

#include <workspace/OpenKAI/OpenKAI/src/AI/_Classifier.h>
#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "../AI/_SegNet.h"
#include "../Autopilot/_AutoPilot.h"
#include "../Camera/CamInput.h"
#include "../Detector/_CascadeDetector.h"
#include "../Detector/_MarkerDetector.h"
#include "../Detector/_BgFgDetector.h"
#include "../Interface/_MavlinkInterface.h"
#include "../UI/UIMonitor.h"
#include "../Interface/_VehicleInterface.h"
#include "../Utility/util.h"
#include "../Image/_DenseFlow.h"

#define APP_NAME "EXT_CAM_CONTROL"

using namespace kai;

namespace kai
{

class ExtCamControl
{
public:
	ExtCamControl();
	~ExtCamControl();

	int m_key;
	bool m_bRun;

	_CamStream* m_pCamFront;
	_AutoPilot* m_pAP;
	_MavlinkInterface* m_pMavlink;
	_MarkerDetector* m_pMD;
	_BgFgDetector* m_pBgFgD;

	CamFrame* m_pShow;
	CamFrame* m_pMat;
	CamFrame* m_pMat2;
	UIMonitor* m_pUIMonitor;

	bool start(JSON* pJson);

	bool showScreenMarkerDetector(void);
	void showScreenBgFgDetector(void);

	void handleMouse(int event, int x, int y, int flags);
	void showInfo(UMat* pDisplayMat);
	void handleKey(int key);
	void printEnvironment(void);

};

}
#endif
