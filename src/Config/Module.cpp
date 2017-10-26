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
	IF_N(!pK);

	ADD_MODULE(_AutoPilot);
	ADD_MODULE(_AprilTags);
	ADD_MODULE(_Automaton);
	ADD_MODULE(_Bullseye);
	ADD_MODULE(_Camera);
	ADD_MODULE(_Canbus);
	ADD_MODULE(_Flow);
	ADD_MODULE(_GPS);
	ADD_MODULE(_ImageNet);
	ADD_MODULE(_Lightware_SF40);
	ADD_MODULE(_LeddarVu);
	ADD_MODULE(_Mavlink);
	ADD_MODULE(_ClusterNet);
	ADD_MODULE(_ZEDobstacle);
	ADD_MODULE(_Path);
	ADD_MODULE(_RC);
	ADD_MODULE(Window);
	ADD_MODULE(_SerialPort);
	ADD_MODULE(_TCPserver);
	ADD_MODULE(_TCPsocket);
	ADD_MODULE(_TCP);
	ADD_MODULE(UDP);
	ADD_MODULE(_UDPclient);
	ADD_MODULE(_UDPserver);

	ADD_MODULE(_BBoxCutOut);
	ADD_MODULE(_Augment);
	ADD_MODULE(_MultiImageNet);

#ifndef USE_OPENCV4TEGRA
	ADD_MODULE(_ROITracker);
#endif
#ifdef USE_CAFFE
	ADD_MODULE(_Caffe);
	ADD_MODULE(_CaffeRegressionTrain);
	ADD_MODULE(_CaffeRegressionInf);
#endif
#ifdef USE_ZED
	ADD_MODULE(_ZED);
	ADD_MODULE(_DNNGen_odometry);
#endif
#ifdef USE_SSD
	ADD_MODULE(_SSD);
#endif
#ifdef USE_FCN
	ADD_MODULE(_FCN);
#endif
#ifdef USE_CASCADE
	ADD_MODULE(_Cascade);
#endif
#ifdef USE_ORB_SLAM2
	ADD_MODULE(_ORB_SLAM2);
#endif
#ifdef USE_LSD_SLAM
	ADD_MODULE(_LSD_SLAM);
#endif
#ifdef USE_TENSORRT
	ADD_MODULE(_DetectNet);
#endif

    return NULL;
}

template <typename T> BASE* Module::createInst(Kiss* pKiss)
{
	IF_N(!pKiss);

	T* pInst = new T();
	if(!pInst->init(pKiss))
	{
		delete pInst;
		return NULL;
	}
    return pInst;
}


Module::Module()
{
}

Module::~Module()
{
}

} /* namespace kai */
