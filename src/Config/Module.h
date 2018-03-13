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

#include "../Detector/_AprilTags.h"
#include "../Detector/_Bullseye.h"
#include "../Detector/_Lane.h"

#include "../include/apriltags-cpp/TagDetector.h"

#include "../Navigation/_GPS.h"
#include "../Navigation/_Path.h"

#include "../Autopilot/_AutoPilot.h"

#include "../IO/_TCPserver.h"
#include "../IO/_TCPsocket.h"
#include "../IO/_TCP.h"
#include "../IO/_File.h"
#include "../IO/_SerialPort.h"
#include "../DNN/TensorRT/_ImageNet.h"
#include "../Detector/_ClusterNet.h"
#include "../DNN/TensorRT/_DetectNet.h"
#include "../Regression/TrainDataGenerator/_DNNGen_odometry.h"
#include "../Sensing/_DNNodometry.h"
#include "../IO/_UDPclient.h"
#include "../IO/_UDPserver.h"
#include "../Protocol/_Canbus.h"
#include "../Protocol/_Mavlink.h"
#include "../Protocol/_RC.h"

#include "../Sensor/_ZEDdistance.h"
#include "../Sensor/_Lightware_SF40_sender.h"
#include "../Sensor/_Lightware_SF40.h"
#include "../Sensor/_LeddarVu.h"
#include "../Sensor/RPLIDAR/_RPLIDAR.h"
#include "../SLAM/_ORB_SLAM2.h"
#include "../Tracker/_ROITracker.h"

#include "../UI/UI.h"
#include "../UI/Window.h"

#include "../Vision/Frame.h"
#include "../Vision/_Camera.h"
#include "../Vision/_GStreamer.h"
#include "../Vision/_ZED.h"
#include "../Vision/FrameGroup.h"
#include "../Vision/_Flow.h"

#include "../Data/Augment/_Augment.h"
#include "../Data/Augment/_filterRotate.h"
#include "../Data/Image/_BBoxCutOut.h"
#include "../Data/Video/_FrameCutOut.h"
#include "../Data/Inference/_MultiImageNet.h"
#include "../IO/_UDP.h"

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
