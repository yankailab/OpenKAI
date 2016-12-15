/*
 * main.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_OPENKAI_MAIN_H_
#define SRC_OPENKAI_MAIN_H_

#include "Application/Startup.h"
#include "IO/File.h"


using namespace kai;

Startup* g_pStart;
Kiss* g_pKiss;
File g_file;

void printEnvironment(void);
void onMouse(int event, int x, int y, int flags, void* userdata);

#endif
