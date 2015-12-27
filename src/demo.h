/*
 * demo.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_DEMO_H_
#define SRC_DEMO_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "AI/_ClassifierManager.h"
#include "Autopilot/_AutoPilot.h"
#include "Detector/MarkerDetector.h"
#include "Camera/CamInput.h"
#include "Detector/_FastDetector.h"
#include "Detector/_ObjectDetector.h"
#include "Interface/_MavlinkInterface.h"
#include "UI/UIMonitor.h"

#include "AI/SegNet.h"

using namespace kai;

int g_key;
bool g_bRun;

_CamStream* g_pCamFront;
_ObjectDetector* g_pOD;
_FastDetector* g_pFD;
_AutoPilot* g_pAP;
_ClassifierManager* g_pClassMgr;
_DenseFlow* g_pDF;
_MavlinkInterface* g_pMavlink;

CamFrame* g_pShow;
UIMonitor* g_pUIMonitor;
MarkerDetector* g_pMD;
cv::Mat g_displayMat;

JSON g_Json;
FileIO g_file;

void onMouse(int event, int x, int y, int flags, void* userdata);
void showScreen(void);
void showInfo(Mat* pDisplayMat);
void handleKey(int key);
void printEnvironment(void);

#endif /* SRC_DEMO_H_ */
