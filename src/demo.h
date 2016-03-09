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

#include "Application/ExtCamControl.h"
#include "Application/SegNetDemo.h"
#include "Application/DroneHunter.h"
#include "Application/Navigator.h"

using namespace kai;

ExtCamControl* g_pAppExtCamControl;
SegNetDemo* g_pAppSegNetDemo;
DroneHunter* g_pAppDroneHunter;
Navigator* g_pAppNavigator;

JSON g_Json;
FileIO g_file;

void printEnvironment(void);
void onMouse(int event, int x, int y, int flags, void* userdata);

#endif /* SRC_DEMO_H_ */
