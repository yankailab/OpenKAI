/*
 * Module.cpp
 *
 *  Created on: Nov 22, 2016
 *      Author: yankai
 */

#include "Module.h"

namespace kai
{

Module::Module()
{
}

Module::~Module()
{
}

BASE* Module::createInstance(Kiss* pK)
{
	IF_N(!pK);

	ADD_MODULE(_AutoPilot);
	ADD_MODULE(_MissionControl);
	ADD_MODULE(_Camera);
	ADD_MODULE(_Canbus);
	ADD_MODULE(_ClusterNet);
	ADD_MODULE(_DepthEdge);
	ADD_MODULE(_GStreamer);
	ADD_MODULE(_Lane);
	ADD_MODULE(_LidarSlam);
	ADD_MODULE(_LeddarVu);
	ADD_MODULE(_LidarOuster);
	ADD_MODULE(_Mavlink);
	ADD_MODULE(_Path);
	ADD_MODULE(PIDctrl);
	ADD_MODULE(_RC);
	ADD_MODULE(_RPLIDAR);
	ADD_MODULE(_Raspivid);
	ADD_MODULE(_SerialPort);
	ADD_MODULE(_TCPserver);
	ADD_MODULE(_TCPclient);
	ADD_MODULE(_UDP);
	ADD_MODULE(_VideoFile);
	ADD_MODULE(_WebSocket);
	ADD_MODULE(Window);

	ADD_MODULE(_Augment);
	ADD_MODULE(_filterBilateralBlur);
	ADD_MODULE(_filterBlur);
	ADD_MODULE(_filterBrightness);
	ADD_MODULE(_filterContrast);
	ADD_MODULE(_filterCrop);
	ADD_MODULE(_filterFlip);
	ADD_MODULE(_filterGaussianBlur);
	ADD_MODULE(_filterHistEqualize);
	ADD_MODULE(_filterLowResolution);
	ADD_MODULE(_filterMedianBlur);
	ADD_MODULE(_filterNoise);
	ADD_MODULE(_filterRotate);
	ADD_MODULE(_filterSaturation);
	ADD_MODULE(_filterShrink);
	ADD_MODULE(_filterTone);

	ADD_MODULE(_BBoxCutOut);
	ADD_MODULE(_FrameCutOut);
	ADD_MODULE(_MultiImageNet);

#ifdef USE_CUDA
	ADD_MODULE(_DenseFlow);
	ADD_MODULE(_Bullseye);
#endif
#ifdef USE_OPENCV_CONTRIB
	ADD_MODULE(_ArUco);
	ADD_MODULE(_SingleTracker);
	ADD_MODULE(_MultiTracker);
	ADD_MODULE(_MotionDetector);
#endif
#ifdef USE_OPENTRACKER
	ADD_MODULE(_EcoTracker);
#endif
#ifdef USE_CAFFE
	ADD_MODULE(_Caffe);
	ADD_MODULE(_CaffeRegressionTrain);
	ADD_MODULE(_CaffeRegressionInf);
#endif
#ifdef USE_SSD
	ADD_MODULE(_SSD);
#endif
#ifdef USE_DARKNET
	ADD_MODULE(_YOLO);
#endif
#ifdef USE_REALSENSE
	ADD_MODULE(_RealSense);
#endif
#ifdef USE_PYLON
	ADD_MODULE(_Pylon);
#endif
#ifdef USE_CASCADE
	ADD_MODULE(_Cascade);
#endif
#ifdef USE_ORB_SLAM2
	ADD_MODULE(_ORB_SLAM2);
#endif
#ifdef USE_TENSORRT
	ADD_MODULE(_ImageNet);
	ADD_MODULE(_DetectNet);
#endif

    return NULL;
}

template <typename T> BASE* Module::createInst(Kiss* pKiss)
{
	IF_N(!pKiss);

	T* pInst = new T();
    return pInst;
}

}
