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

	ADD_MODULE(_ANR);
	ADD_MODULE(_ProtocolBase);
	ADD_MODULE(_MissionControl);
	ADD_MODULE(_Camera);
	ADD_MODULE(_Canbus);
	ADD_MODULE(_Contrast);
	ADD_MODULE(_Crop);
	ADD_MODULE(_CETCUS);
	ADD_MODULE(_DNNclassifier);
	ADD_MODULE(_DNNdetect);
	ADD_MODULE(_DNNtext);
	ADD_MODULE(_Depth2Gray);
	ADD_MODULE(_DepthSegment);
	ADD_MODULE(_Erode);
	ADD_MODULE(_GStreamer);
	ADD_MODULE(_GPS);
	ADD_MODULE(_Grayscale);
	ADD_MODULE(_GDcam);
	ADD_MODULE(_HiphenRGB);
	ADD_MODULE(_HiphenCMD);
	ADD_MODULE(_HiphenServer);
	ADD_MODULE(_HistEqualize);
	ADD_MODULE(_Invert);
	ADD_MODULE(_IRLock);
	ADD_MODULE(_InRange);
	ADD_MODULE(_ImgFile);
	ADD_MODULE(_Lane);
	ADD_MODULE(_Line);
	ADD_MODULE(_SlideWindow);
	ADD_MODULE(_LidarSlam);
	ADD_MODULE(_LeddarVu);
	ADD_MODULE(_Morphology);
	ADD_MODULE(_Mavlink);
	ADD_MODULE(_MOAB);
	ADD_MODULE(_Modbus);
	ADD_MODULE(_Path);
	ADD_MODULE(PIDctrl);
	ADD_MODULE(_Object);
	ADD_MODULE(_ObjectFactory);
	ADD_MODULE(_OpenPose);
	ADD_MODULE(_OrientalMotor);
	ADD_MODULE(_RPLIDAR);
	ADD_MODULE(_Raspivid);
	ADD_MODULE(_Resize);
	ADD_MODULE(_Rotate);
	ADD_MODULE(_SerialPort);
	ADD_MODULE(_ShopCam);
	ADD_MODULE(_TCPserver);
	ADD_MODULE(_TCPclient);
	ADD_MODULE(_Thermal);
	ADD_MODULE(_Threshold);
	ADD_MODULE(_UDP);
	ADD_MODULE(_Universe);
	ADD_MODULE(_VideoFile);
	ADD_MODULE(_WebSocket);
	ADD_MODULE(Window);
	ADD_MODULE(Console);

	ADD_MODULE(_ArduServo);
	ADD_MODULE(_ActuatorSync);
	ADD_MODULE(_Sequencer);
	ADD_MODULE(_SortingArm);
	ADD_MODULE(_SortingCtrlServer);
	ADD_MODULE(_SortingCtrlClient);

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

	ADD_MODULE(_GDimgUploader);
	ADD_MODULE(_BBoxCutOut);
	ADD_MODULE(_CutOut);
	ADD_MODULE(_FrameCutOut);
	ADD_MODULE(_MultiImageNet);

	ADD_MODULE(_AP_base);
	ADD_MODULE(_AP_depthVision);
	ADD_MODULE(_AP_distLidar);
	ADD_MODULE(_AP_avoid);
	ADD_MODULE(_AP_follow);
	ADD_MODULE(_AP_land);
	ADD_MODULE(_AP_mission);
	ADD_MODULE(_AP_posCtrl);
	ADD_MODULE(_AP_RTH);
	ADD_MODULE(_AP_servo);
	ADD_MODULE(_AP_slam);
	ADD_MODULE(_AP_takeoff);
	ADD_MODULE(_AP_followClient);
	ADD_MODULE(_AP_WP);
	ADD_MODULE(_AP_link);

	ADD_MODULE(_Rover_avoid);
	ADD_MODULE(_Rover_base);
	ADD_MODULE(_Rover_CMD);
	ADD_MODULE(_Rover_drive);
	ADD_MODULE(_Rover_field);
	ADD_MODULE(_Rover_follow);
	ADD_MODULE(_Rover_WP);

#ifdef USE_MYNTEYE
	ADD_MODULE(_Mynteye);
#endif
#ifdef USE_LIVOX
	ADD_MODULE(_Livox);
#endif
#ifdef USE_DYNAMIXEL
	ADD_MODULE(_DeltaArm);
#endif
#ifdef USE_OCR
	ADD_MODULE(OCR);
#endif
#ifdef USE_OPENALPR
	ADD_MODULE(_OpenALPR);
#endif
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
#ifdef USE_DARKNET
	ADD_MODULE(_YOLO);
#endif
#ifdef USE_REALSENSE
	ADD_MODULE(_RealSense);
	ADD_MODULE(_RStracking);
	ADD_MODULE(_AP_takePhoto);
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
