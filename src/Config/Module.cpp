/*
 * Module.cpp
 *
 *  Created on: Nov 22, 2016
 *      Author: root
 */

#include "Module.h"

namespace kai
{

BASE* Module::createInstance(Kiss* pK)
{
	CHECK_N(!pK);

	ADD_MODULE(_Stream);
	ADD_MODULE(_Automaton);
	ADD_MODULE(_Universe);
	ADD_MODULE(_Mavlink);
	ADD_MODULE(_Canbus);
	ADD_MODULE(_RC);
	ADD_MODULE(_AutoPilot);
	ADD_MODULE(_AprilTags);
	ADD_MODULE(_Bullseye);
	ADD_MODULE(_ROITracker);
	ADD_MODULE(_Flow);
	ADD_MODULE(_Depth);
	ADD_MODULE(_Lightware_SF40);
	ADD_MODULE(_server);
	ADD_MODULE(_peer);

#ifdef USE_SSD
	ADD_MODULE(_SSD);
#endif
#ifdef USE_FCN
	ADD_MODULE(_FCN);
#endif
#ifdef USE_TENSORRT
	ADD_MODULE(_DetectNet);
#endif

    return NULL;
}

template <typename T> BASE* Module::createInst(Kiss* pKiss)
{
	CHECK_N(!pKiss);

	T* pInst = new T();
	CHECK_N(!pInst->init(pKiss));
    return pInst;
}


Module::Module()
{
}

Module::~Module()
{
}

} /* namespace kai */
