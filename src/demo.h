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

#include "Detector/ObjectDetector.h"
#include "Detector/FastDetector.h"
#include "Detector/MarkerDetector.h"
#include "Camera/CamInput.h"
#include "UI/UIMonitor.h"
#include "Autopilot/AutoPilot.h"
#include "Interface/MavlinkInterface.h"

using namespace kai;

int g_key;
bool g_bRun;

CamStream* g_pCamFront;
UIMonitor* g_pUIMonitor;
ObjectDetector* g_pOD;
FastDetector* g_pFD;
AutoPilot* g_pAP;
MarkerDetector* g_pMD;

MavlinkInterface* g_pMavlink;
cv::Mat g_displayMat;

JSON g_Json;
FileIO g_file;
string g_serialPort;
int g_baudrate;

void onMouse(int event, int x, int y, int flags, void* userdata);
void showScreen(Mat* pDisplayMat);
void showInfo(Mat* pDisplayMat);
void handleKey(int key);
void printEnvironment(void);






#endif /* SRC_DEMO_H_ */
