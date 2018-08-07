/*
 * main.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_main_H_
#define OpenKAI_src_main_H_

#include "Startup/Startup.h"
#include "IO/_File.h"


using namespace kai;

Startup* g_pStart;
Kiss* g_pKiss;
_File g_file;

void printEnvironment(void);
void onMouse(int event, int x, int y, int flags, void* userdata);

#endif
