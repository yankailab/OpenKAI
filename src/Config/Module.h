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

#include "../Actuator/_OrientalMotor.h"
#include "../Actuator/_ArduServo.h"
#include "../Actuator/_ActuatorSync.h"

#include "../Application/Autopilot/APcopter/_APcopter_base.h"
#include "../Application/Autopilot/APcopter/_APcopter_depthVision.h"
#include "../Application/Autopilot/APcopter/_APcopter_distLidar.h"
#include "../Application/Autopilot/APcopter/_APcopter_avoid.h"
#include "../Application/Autopilot/APcopter/_APcopter_land.h"
#include "../Application/Autopilot/APcopter/_APcopter_posCtrl.h"
#include "../Application/Autopilot/APcopter/_APcopter_RTH.h"
#include "../Application/Autopilot/APcopter/_APcopter_servo.h"
#include "../Application/Autopilot/APcopter/_APcopter_slam.h"
#include "../Application/Autopilot/APcopter/_APcopter_followClient.h"
#include "../Application/Autopilot/APcopter/_APcopter_takePhoto.h"
#include "../Application/Autopilot/APcopter/_APcopter_follow.h"
#include "../Application/Autopilot/APcopter/_APcopter_WP.h"
#include "../Application/Autopilot/APcopter/_APcopter_link.h"

#include "../Application/Autopilot/Rover/_Rover_avoid.h"
#include "../Application/Autopilot/Rover/_Rover_base.h"
#include "../Application/Autopilot/Rover/_Rover_CMD.h"
#include "../Application/Autopilot/Rover/_Rover_drive.h"
#include "../Application/Autopilot/Rover/_Rover_field.h"
#include "../Application/Autopilot/Rover/_Rover_follow.h"
#include "../Application/Autopilot/Rover/_Rover_WP.h"

#include "../Application/Observation/_HiphenRGB.h"
#include "../Application/Surveillance/_ANR.h"
#include "../Application/Surveillance/_GDcam.h"
#include "../Application/Surveillance/_ShopCam.h"
#include "../Application/Automation/Sorting/_Sequencer.h"
#include "../Application/Automation/Sorting/_SortingArm.h"
#include "../Application/Automation/Sorting/_SortingCtrlServer.h"
#include "../Application/Automation/Sorting/_SortingCtrlClient.h"
#include "../Application/Mission/_MissionControl.h"

#include "../Control/PIDctrl.h"
#include "../Detector/_ArUco.h"
#include "../Detector/_Lane.h"
#include "../Detector/_MotionDetector.h"
#include "../Detector/_DNNdetect.h"
#include "../Detector/_DNNclassifier.h"
#include "../Detector/_DNNtext.h"
#include "../Detector/_Line.h"
#include "../Detector/_Thermal.h"
#include "../Detector/_DepthSegment.h"
#include "../Detector/_OpenPose.h"
#include "../Detector/_IRLock.h"

#include "../Navigation/_Path.h"
#include "../Navigation/_GPS.h"

#include "../IO/_TCPserver.h"
#include "../IO/_TCPclient.h"
#include "../IO/_File.h"
#include "../IO/_SerialPort.h"
#include "../IO/_UDP.h"
#include "../IO/_WebSocket.h"
#include "../DNN/TensorRT/_ImageNet.h"
#include "../DNN/TensorRT/_DetectNet.h"
#include "../Protocol/_Canbus.h"
#include "../Protocol/_Mavlink.h"
#include "../Protocol/_MOAB.h"
#include "../Protocol/_ProtocolBase.h"
#include "../Protocol/_Modbus.h"
#include "../Sensor/_LeddarVu.h"
#include "../Sensor/RPLIDAR/_RPLIDAR.h"
#include "../SLAM/_ORB_SLAM2.h"
#include "../SLAM/_LidarSlam.h"

#include "../Tracker/_SingleTracker.h"
#include "../Tracker/_MultiTracker.h"
#include "../Tracker/_EcoTracker.h"

#include "../UI/Window.h"
#include "../UI/Console.h"

#include "../Universe/_Object.h"
#include "../Universe/_ObjectFactory.h"
#include "../Universe/_Universe.h"

#include "../Vision/Frame.h"
#include "../Vision/_Camera.h"
#include "../Vision/_VideoFile.h"
#include "../Vision/_ImgFile.h"
#include "../Vision/_GStreamer.h"
#include "../Vision/_Raspivid.h"
#include "../Vision/FrameGroup.h"
#include "../Vision/Hiphen/_HiphenServer.h"
#include "../Vision/Hiphen/_HiphenCMD.h"
#include "../Vision/ImgFilter/_Morphology.h"
#include "../Vision/ImgFilter/_Threshold.h"
#include "../Vision/ImgFilter/_Contrast.h"
#include "../Vision/ImgFilter/_Crop.h"
#include "../Vision/ImgFilter/_Depth2Gray.h"
#include "../Vision/ImgFilter/_Erode.h"
#include "../Vision/ImgFilter/_Grayscale.h"
#include "../Vision/ImgFilter/_HistEqualize.h"
#include "../Vision/ImgFilter/_Invert.h"
#include "../Vision/ImgFilter/_InRange.h"
#include "../Vision/ImgFilter/_Resize.h"
#include "../Vision/ImgFilter/_Rotate.h"

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

#include "../Data/Image/_GDimgUploader.h"
#include "../Data/Image/_BBoxCutOut.h"
#include "../Data/Image/_CutOut.h"
#include "../Data/Video/_FrameCutOut.h"
#include "../Data/Inference/_MultiImageNet.h"
#include "../Detector/_SlideWindow.h"

#ifdef USE_MYNTEYE
#include "../Vision/_Mynteye.h"
#endif
#ifdef USE_DYNAMIXEL
#include "../Actuator/_DeltaArm.h"
#endif
#ifdef USE_OCR
#include "../Detector/OCR.h"
#endif
#ifdef USE_OPENALPR
#include "../Detector/_OpenALPR.h"
#endif
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
#ifdef USE_DARKNET
#include "../DNN/Darknet/_YOLO.h"
#endif
#ifdef USE_REALSENSE
#include "../Vision/_RealSense.h"
#include "../SLAM/_RStracking.h"
#endif
#ifdef USE_PYLON
#include "../Vision/_Pylon.h"
#endif
#ifdef USE_LIVOX
#include "../Sensor/_Livox.h"
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
