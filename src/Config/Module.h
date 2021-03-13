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

#include "../Actuator/_ArduServo.h"
#include "../Actuator/_ActuatorSync.h"
#include "../Actuator/_DRV8825_RS485.h"
#include "../Actuator/_OrientalMotor.h"
#include "../Actuator/Articulated/_S6H4D.h"
#include "../Actuator/_StepperGripper.h"
#include "../Actuator/Motor/_ZLAC8015.h"

#include "../Application/Autopilot/ArduPilot/_AP_base.h"
#include "../Application/Autopilot/ArduPilot/_AP_actuator.h"
#include "../Application/Autopilot/ArduPilot/_AP_distLidar.h"
#include "../Application/Autopilot/ArduPilot/_AP_goto.h"
#include "../Application/Autopilot/ArduPilot/_AP_gcs.h"
#include "../Application/Autopilot/ArduPilot/_AP_GPS.h"
#include "../Application/Autopilot/ArduPilot/_AP_link.h"
#include "../Application/Autopilot/ArduPilot/_AP_mission.h"
#include "../Application/Autopilot/ArduPilot/_AP_posCtrl.h"
#include "../Application/Autopilot/ArduPilot/_AP_RTH.h"
#include "../Application/Autopilot/ArduPilot/_AP_relay.h"
#include "../Application/Autopilot/ArduPilot/_AP_servo.h"
#include "../Application/Autopilot/ArduPilot/_AP_takeoff.h"
#include "../Application/Autopilot/ArduPilot/_AProver_drive.h"
#include "../Application/Autopilot/ArduPilot/_AProver_picking.h"
#include "../Application/RobotArm/_Sequencer.h"

#include "../Application/UTphenoRover/_AProver_UT.h"
#include "../Application/UTphenoRover/_UTprArmL.h"
#include "../Application/UTphenoRover/_AProver_UTfollowTag.h"
#include "../Application/KUphenoRover/_AProver_KU.h"
#include "../Application/KUphenoRover/_AProver_KUfollowTag.h"
#include "../Application/BenchRover/_AProver_BR.h"
#include "../Application/BenchRover/_AProver_BRfollow.h"
#include "../Application/TagRover/_AProver_tag.h"
#include "../Application/TagRover/_AProver_followTag.h"
#include "../Application/DroneBox/_DroneBox.h"
#include "../Application/DroneBox/_DroneBoxJSON.h"
#include "../Application/DroneBox/_AP_droneBoxJSON.h"

#include "../Control/PID.h"
#include "../Control/_Drive.h"

#include "../State/_StateControl.h"

#include "../Navigation/_Path.h"
#include "../Navigation/_GPS.h"
#include "../Navigation/_RTCM3.h"

#include "../IO/_TCPserver.h"
#include "../IO/_TCPclient.h"
#include "../IO/_File.h"
#include "../IO/_SerialPort.h"
#include "../IO/_UDP.h"
#include "../IO/_WebSocket.h"
#include "../Protocol/_Canbus.h"
#include "../Protocol/_CETCUS.h"
#include "../Protocol/_Mavlink.h"
#include "../Protocol/_MOAB.h"
#include "../Protocol/_ProtocolBase.h"
#include "../Protocol/_Modbus.h"
#include "../Sensor/_LeddarVu.h"
#include "../Sensor/_TOFsense.h"
#include "../Sensor/RPLIDAR/_RPLIDAR.h"

#include "../UI/Console.h"

#include "../Universe/_Object.h"
#include "../Universe/_Universe.h"

#include "../Vision/Hiphen/_HiphenServer.h"
#include "../Vision/Hiphen/_HiphenCMD.h"

#ifdef USE_OPENCL
#include "../Compute/OpenCL/clBase.h"
#endif

#ifdef USE_OPENCV

#include "../UI/Window.h"
#include "../Vision/_Camera.h"
#include "../Vision/_VideoFile.h"
#include "../Vision/_ImgFile.h"
#include "../Vision/_GStreamer.h"
#include "../Vision/_Raspivid.h"
#include "../Vision/_GPhoto.h"
#include "../Vision/ImgFilter/_Morphology.h"
#include "../Vision/ImgFilter/_Mask.h"
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

#include "../Data/Image/_GDimgUploader.h"
#include "../Data/Image/_BBoxCutOut.h"
#include "../Data/Image/_CutOut.h"
#include "../Data/Video/_FrameCutOut.h"

#include "../Detector/_Lane.h"
#include "../Detector/_DNNdetect.h"
#include "../Detector/_DNNclassifier.h"
#include "../Detector/_DNNtext.h"
#include "../Detector/_DepthSegment.h"
#include "../Detector/_IRLock.h"
#include "../Detector/_Line.h"
#include "../Detector/_OpenPose.h"
#include "../Detector/_HandKey.h"
#include "../Detector/_SlideWindow.h"
#include "../Detector/_Thermal.h"

#include "../Application/Autopilot/ArduPilot/_AP_avoid.h"
#include "../Application/Autopilot/ArduPilot/_AP_depthVision.h"
#include "../Application/Autopilot/ArduPilot/_AP_land.h"
#include "../Application/Autopilot/ArduPilot/_AP_follow.h"
#include "../Application/Autopilot/ArduPilot/_APcopter_photo.h"

#include "../Application/Observation/_HiphenRGB.h"
#include "../Application/Surveillance/_ANR.h"
#include "../Application/Surveillance/_GDcam.h"
#include "../Application/Surveillance/_ShopCam.h"

#include "../Application/RobotArm/_PickingArm.h"
#include "../Application/RobotArm/_SortingArm.h"
#include "../Application/RobotArm/_SortingCtrlServer.h"
#include "../Application/RobotArm/_SortingCtrlClient.h"

#ifdef USE_OPENCV_CONTRIB
#include "../Detector/_ArUco.h"
#include "../Detector/_MotionDetector.h"
#include "../Tracker/_SingleTracker.h"
#endif

#ifdef USE_CUDA
#include "../Detector/_Bullseye.h"
#include "../Detector/_Cascade.h"
#include "../Vision/_DenseFlow.h"
#endif

#ifdef USE_REALSENSE
#include "../Vision/_RealSense.h"
#endif

#ifdef USE_MYNTEYE
#include "../Vision/_Mynteye.h"
#endif

#ifdef USE_INNFOS
#include "../Actuator/_InnfosGluon.h"
#endif

#ifdef USE_JETSON_INFERENCE
#include "../DNN/JetsonInference/_ImageNet.h"
#include "../DNN/JetsonInference/_DetectNet.h"
#endif

#ifdef USE_OCR
#include "../Detector/OCR.h"
#endif

#ifdef USE_OPENALPR
#include "../Detector/_OpenALPR.h"
#endif

#ifdef USE_CAFFE
#include "../DNN/Caffe/_Caffe.h"
#include "../Regression/_CaffeRegressionTrain.h"
#include "../Regression/_CaffeRegressionInf.h"
#endif

#ifdef USE_DARKNET
#include "../DNN/Darknet/_YOLO.h"
#endif

#ifdef USE_PYLON
#include "../Vision/_Pylon.h"
#endif

#ifdef USE_CHILITAGS
#include "../Detector/_Chilitags.h"
#endif

#endif	//USE_OPENCV

#ifdef USE_OPEN3D
#include "../PointCloud/_PCfile.h"
#include "../PointCloud/PCfilter/_PCcrop.h"
#include "../PointCloud/PCfilter/_PCremove.h"
#include "../PointCloud/PCfilter/_PCdownSample.h"
#include "../PointCloud/_PCmerge.h"
#include "../PointCloud/PCregistration/_PCregistCol.h"
#include "../PointCloud/PCregistration/_PCregistICP.h"
#include "../PointCloud/PCregistration/_PCregistGlobal.h"
#include "../PointCloud/_PCtransform.h"
#include "../PointCloud/_PCviewer.h"
#include "../PointCloud/_PCsend.h"
#include "../PointCloud/_PCrecv.h"
#include "../PointCloud/_PClattice.h"
#include "../Application/3Dscan/_PCscan.h"
#ifdef USE_REALSENSE
#include "../PointCloud/_PCrs.h"
#endif
#endif

#ifdef USE_HPS3D
#include "../PointCloud/_HPS3D.h"
#endif

#ifdef USE_REALSENSE
#include "../SLAM/_RStracking.h"
#endif

#ifdef USE_DYNAMIXEL
#include "../Actuator/_DeltaArm.h"
#include "../Actuator/_LabArm.h"
#endif

#ifdef USE_LIVOX
#include "../Sensor/Livox/_Livox.h"
#include "../Sensor/Livox/LivoxLidar.h"
#endif

#ifdef USE_XARM
#include "../Actuator/Articulated/_xArm.h"
#endif

#ifdef USE_ORB_SLAM
#include "../SLAM/_ORB_SLAM.h"
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
