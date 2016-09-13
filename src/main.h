/*
 * main.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_OPENKAI_MAIN_H_
#define SRC_OPENKAI_MAIN_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "Application/General.h"

using namespace kai;

General* g_pGen;

Config* g_pConfig;
FileIO g_file;

void printEnvironment(void);
void onMouse(int event, int x, int y, int flags, void* userdata);

#endif /* SRC_OPENKAI_MAIN_H_ */
