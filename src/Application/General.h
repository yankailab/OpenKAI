/*
 * General.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_GENERAL_H_
#define SRC_GENERAL_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "../Base/common.h"
#ifdef USE_SSD
#include "../AI/_SSD.h"
#endif
#ifdef USE_FCN
#include "../AI/_FCN.h"
#endif
#ifdef USE_TENSORRT
#include "../AI/_DetectNet.h"
#endif

#include "../Base/BASE.h"
#include "../Automaton/_Automaton.h"
#include "../Autopilot/_AutoPilot.h"
#include "../Camera/Camera.h"
#include "../Interface/_Mavlink.h"
#include "../Interface/_Canbus.h"
#include "../Detector/_Bullseye.h"
#include "../Detector/_AprilTags.h"
#include "../Tracker/_ROITracker.h"
#include "../Detector/_Depth.h"
#include "../Navigation/_Universe.h"
#include "../UI/UI.h"
#include "../Vision/_Flow.h"
#include "AppBase.h"

#define N_INST 16

using namespace kai;

namespace kai
{

class General: public AppBase
{
public:
	General();
	~General();

	bool start(Config* pConfig);
	void draw(void);
	void handleMouse(int event, int x, int y, int flags);
	void handleKey(int key);

	bool createAllInst(Config* pConfig);
	BASE* getInstByName(string* pName);
	bool linkAllInst(Config* pConfig);

	template <typename T> bool createInst(Config* pConfig);

public:
	Frame*	m_pFrame;

	int		m_nInst;
	BASE* 	m_pInst[N_INST];

	int 	m_nMouse;
	BASE* 	m_pMouse[N_INST];

};

}
#endif
