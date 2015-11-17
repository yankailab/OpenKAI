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
#include "Camera/CamInput.h"
#include "Camera/CamMarkerDetect.h"
#include "Camera/CamMonitor.h"
#include "Autopilot/AutoPilot.h"
#include "IO/Config.h"
#include "Interface/MavlinkInterface.h"

using namespace kai;

int g_key;
bool g_bRun;

CamStream* g_pCamFront;
ObjectDetector* g_pOD;
FastDetector* g_pFD;
AutoPilot* g_pAP;
CamMarkerDetect* g_pMD;

MavlinkInterface* g_pMavlink;
cv::Mat g_displayMat;

JSON g_Json;
FileIO g_file;
Config g_config;
string g_serialPort;
int g_baudrate;

void onMouse(int event, int x, int y, int flags, void* userdata);
void createConfigWindow(void);
void onTrackbar(int, void*);
void displayInfo(Mat* pDisplayMat);
void handleKey(int key);
void printEnvironment(void);





#endif /* SRC_DEMO_H_ */
