/*
 * Module.h
 *
 *  Created on: Nov 22, 2016
 *      Author: root
 */

#ifndef OpenKAI_src_Config_Module_H_
#define OpenKAI_src_Config_Module_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Script/Kiss.h"
#include "switch.h"

#include "../Automaton/_Automaton.h"
#include "../Detector/_ArUco.h"
#include "../Detector/_DepthEdge.h"
#include "../Detector/_Lane.h"
#include "../Detector/_MotionDetector.h"

#include "../Navigation/_Path.h"

#include "../Autopilot/_AutoPilot.h"
#include "../Control/PIDctrl.h"

#include "../IO/_TCPserver.h"
#include "../IO/_TCPclient.h"
#include "../IO/_File.h"
#include "../IO/_SerialPort.h"
#include "../IO/_UDP.h"
#include "../IO/_WebSocket.h"
#include "../DNN/TensorRT/_ImageNet.h"
#include "../Detector/_ClusterNet.h"
#include "../DNN/TensorRT/_DetectNet.h"
#include "../Protocol/_Canbus.h"
#include "../Protocol/_Mavlink.h"
#include "../Protocol/_RC.h"

#include "../Sensor/_LeddarVu.h"
#include "../Sensor/RPLIDAR/_RPLIDAR.h"
#include "../SLAM/_ORB_SLAM2.h"
#include "../SLAM/_LidarSlam.h"

#include "../Tracker/_SingleTracker.h"
#include "../Tracker/_MultiTracker.h"
#include "../Tracker/_EcoTracker.h"

#include "../UI/Window.h"

#include "../Vision/Frame.h"
#include "../Vision/_Camera.h"
#include "../Vision/_VideoFile.h"
#include "../Vision/_GStreamer.h"
#include "../Vision/_Raspivid.h"
#include "../Vision/FrameGroup.h"

#include "../Data/Augment/_Augment.h"
#include "../Data/Augment/_filterBilateralBlur.h"
#include "../Data/Augment/_filterBlur.h"
#include "../Data/Augment/_filterBrightness.h"
#include "../Data/Augment/_filterContrast.h"
#include "../Data/Augment/_filterCrop.h"
#include "../Data/Augment/_filterFlip.h"
#include "../Data/Augment/_filterGaussianBlur.h"
#include "../Data/Augment/_filterHistEqualize.h"
#include "../Data/Augment/_filterLowResolution.h"
#include "../Data/Augment/_filterMedianBlur.h"
#include "../Data/Augment/_filterNoise.h"
#include "../Data/Augment/_filterRotate.h"
#include "../Data/Augment/_filterSaturation.h"
#include "../Data/Augment/_filterShrink.h"
#include "../Data/Augment/_filterTone.h"

#include "../Data/Image/_BBoxCutOut.h"
#include "../Data/Video/_FrameCutOut.h"
#include "../Data/Inference/_MultiImageNet.h"

#ifdef USE_CUDA
#include "../Vision/_DenseFlow.h"
#include "../Detector/_Bullseye.h"
#endif
#ifdef USE_CASCADE
#include "../Detector/_Cascade.h"
#endif
#ifdef USE_CAFFE
#include "../DNN/Caffe/_Caffe.h"
#include "../Regression/_CaffeRegressionTrain.h"
#include "../Regression/_CaffeRegressionInf.h"
#endif
#ifdef USE_SSD
#include "../DNN/SSD/_SSD.h"
#endif
#ifdef USE_DARKNET
#include "../DNN/Darknet/_YOLO.h"
#endif
#ifdef USE_REALSENSE
#include "../Vision/_RealSense.h"
#endif
#ifdef USE_PYLON
#include "../Vision/_Pylon.h"
#endif

#define ADD_MODULE(x) if(pK->m_class == #x){return createInst<x>(pK);}

namespace kai
{

class Module
{
public:
	Module();
	virtual ~Module();
	BASE* createInstance(Kiss* pK);
private:
	template <typename T> BASE* createInst(Kiss* pKiss);
};

}

#endif
