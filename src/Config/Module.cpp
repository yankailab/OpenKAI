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

	BASE *Module::createInstance(Kiss *pK)
	{
		IF_N(!pK);

		ADD_MODULE(_SharedMem);
		ADD_MODULE(Destimator);
		ADD_MODULE(_Console);

#ifdef WITH_ACTUATOR
		ADD_MODULE(_ArduServo);
		ADD_MODULE(_ActuatorSync);
		ADD_MODULE(_OrientalMotor);
		ADD_MODULE(_HYMCU_RS485);
		ADD_MODULE(_S6H4D);
		ADD_MODULE(_StepperGripper);
		ADD_MODULE(_ZLAC8015);
		ADD_MODULE(_ZLAC8015D);
#ifdef USE_XARM
		ADD_MODULE(_xArm);
#endif
#endif

#ifdef WITH_ARDUPILOT
		ADD_MODULE(_AP_base);
		ADD_MODULE(_AP_distLidar);
		ADD_MODULE(_AP_actuator);
		ADD_MODULE(_AP_mission);
		ADD_MODULE(_AP_posCtrl);
		ADD_MODULE(_AP_RTH);
		ADD_MODULE(_AP_relay);
		ADD_MODULE(_AP_servo);
		ADD_MODULE(_AP_takeoff);
		ADD_MODULE(_AP_goto);
		ADD_MODULE(_AP_gcs);
		ADD_MODULE(_AP_link);
		ADD_MODULE(_AProver_drive);
#ifdef USE_OPENCV
		ADD_MODULE(_AP_avoid);
		ADD_MODULE(_AP_depthVision);
		ADD_MODULE(_AP_land);
		ADD_MODULE(_AP_follow);
		ADD_MODULE(_AP_video);
		ADD_MODULE(_APcopter_photo);
#ifdef WITH_APP_ROBOTARM
		ADD_MODULE(_AProver_picking);
#endif
#ifdef WITH_NAVIGATION
#ifdef USE_REALSENSE
		ADD_MODULE(_AP_GPS);
#endif
#endif
#endif
#endif

#ifdef WITH_AUTOPILOT_DRIVE
		ADD_MODULE(_Drive);
#endif

#ifdef WITH_APP_MULTI3DSCAN
#endif

#ifdef WITH_APP_BENCHROVER
		ADD_MODULE(_AProver_BR);
		ADD_MODULE(_AProver_BRfollow);
#endif

#ifdef WITH_APP_CAMCALIB
		ADD_MODULE(_CamCalib);
#endif

#ifdef WITH_APP_DEPTHCAM
		ADD_MODULE(_DepthCam);
#endif

#ifdef WITH_APP_DRONEBOX
		ADD_MODULE(_DroneBox);
		ADD_MODULE(_DroneBoxJSON);
		ADD_MODULE(_AP_droneBoxJSON);
#endif

#ifdef WITH_APP_KUPHENOROVER
		ADD_MODULE(_AProver_KU);
		ADD_MODULE(_AProver_KUfollowTag);
#endif

#ifdef WITH_APP_MEASUREMENT
		ADD_MODULE(_ARmeasure);
		ADD_MODULE(_ARmeasureVertex);
		ADD_MODULE(_ARmeasureFree);
		ADD_MODULE(_ARmeasureCalibCam);
		ADD_MODULE(_ARmeasureCalibDofs);
		ADD_MODULE(_RaspiWSbattery);
#endif

#ifdef WITH_APP_3DSCAN
#ifdef USE_OPEN3D
		ADD_MODULE(_3DScanCalibCam);
		ADD_MODULE(_3DScanCalibOfs);
		ADD_MODULE(_PCscan);
		ADD_MODULE(_PCcalib);
#endif
#endif

#ifdef WITH_APP_ROBOTARM
		ADD_MODULE(_Sequencer);
#ifdef USE_OPENCV
		ADD_MODULE(_PickingArm);
#endif
#endif

#ifdef WITH_APP_ROVER
		ADD_MODULE(_PWMrover);
		ADD_MODULE(_RCrover);
#ifdef USE_OPENCV
#endif
#endif

#ifdef WITH_APP_SURVEILLANCE
#ifdef USE_OPENCV
		ADD_MODULE(_ANR);
		ADD_MODULE(_GDcam);
#endif
#endif

#ifdef WITH_APP_TAGROVER
		ADD_MODULE(_AProver_followTag);
		ADD_MODULE(_AProver_tag);
#endif

#ifdef WITH_APP_UTPHENOROVER
		ADD_MODULE(_AProver_UT);
		ADD_MODULE(_AProver_UTfollowTag);
		ADD_MODULE(_UTprArmL);
#endif

#ifdef WITH_COMPUTE
#ifdef USE_OPENCL
		ADD_MODULE(clBase);
#endif
#endif

#ifdef WITH_CONTROL
		ADD_MODULE(PID);
#endif

#ifdef WITH_DATA
#ifdef USE_OPENCV
		ADD_MODULE(_GDimgUploader);
		ADD_MODULE(_BBoxCutOut);
		ADD_MODULE(_CutOut);
		ADD_MODULE(_FrameCutOut);
#endif
#endif

#ifdef WITH_DETECTOR
#ifdef USE_OPENCV
		ADD_MODULE(_DNNclassifier);
		ADD_MODULE(_DNNdetect);
		ADD_MODULE(_DNNtext);
		ADD_MODULE(_DepthSegment);
		ADD_MODULE(_IRLock);
		ADD_MODULE(_OpenPose);
		ADD_MODULE(_HandKey);
		ADD_MODULE(_Lane);
		ADD_MODULE(_Line);
		ADD_MODULE(_SlideWindow);
		ADD_MODULE(_Thermal);
#ifdef USE_OPENCV_CONTRIB
		ADD_MODULE(_ArUco);
		ADD_MODULE(_MotionDetector);
		ADD_MODULE(_SingleTracker);
#endif
#ifdef USE_CUDA
		ADD_MODULE(_Bullseye);
		ADD_MODULE(_Cascade);
#endif
#ifdef USE_OCR
		ADD_MODULE(OCR);
#endif
#ifdef USE_OPENALPR
		ADD_MODULE(_OpenALPR);
#endif
#ifdef USE_CHILITAGS
		ADD_MODULE(_Chilitags);
#endif
#endif //USE_OPENCV
#endif

#ifdef WITH_DNN
#ifdef USE_OPENCV
#ifdef USE_JETSON_INFERENCE
		//	ADD_MODULE(_ImageNet);
		ADD_MODULE(_DetectNet);
#endif
#ifdef USE_CAFFE
		ADD_MODULE(_Caffe);
		ADD_MODULE(_CaffeRegressionTrain);
		ADD_MODULE(_CaffeRegressionInf);
#endif
#ifdef USE_DARKNET
		ADD_MODULE(_YOLO);
#endif
#endif
#endif

#ifdef WITH_FILTER
#endif

#ifdef WITH_GCS
#endif

#ifdef WITH_IO
		ADD_MODULE(_SerialPort);
		ADD_MODULE(_TCPserver);
		ADD_MODULE(_TCPclient);
		ADD_MODULE(_UDP);
		ADD_MODULE(_WebSocket);
#endif

#ifdef WITH_NAVIGATION
		ADD_MODULE(_RTCM3);
		ADD_MODULE(_GPS);
#ifdef USE_REALSENSE
		ADD_MODULE(_RStracking);
#endif
#endif

#ifdef WITH_NET
		ADD_MODULE(_Curl);
#endif

#ifdef WITH_3D
#ifdef USE_OPEN3D
		ADD_MODULE(_GeometryViewer);
		ADD_MODULE(_MeshStream);
		ADD_MODULE(_PCstream);
		ADD_MODULE(_PCframe);
		ADD_MODULE(_PClattice);
		ADD_MODULE(_PCfile);
		ADD_MODULE(_PCmerge);
		ADD_MODULE(_PCsend);
		ADD_MODULE(_PCrecv);
		ADD_MODULE(_PCtransform);
		ADD_MODULE(_PCcrop);
		ADD_MODULE(_PCremove);
		ADD_MODULE(_PCdownSample);
		ADD_MODULE(_PCregistCol);
		ADD_MODULE(_PCregistICP);
		ADD_MODULE(_PCregistGlobal);
#ifdef USE_REALSENSE
		ADD_MODULE(_PCrs);
#endif
#ifdef USE_HPS3D
		ADD_MODULE(_HPS3D);
#endif
#endif
#endif

#ifdef WITH_PROTOCOL
		ADD_MODULE(_Canbus);
		ADD_MODULE(_MOAB);
		ADD_MODULE(_Mavlink);
		ADD_MODULE(_Modbus);
		ADD_MODULE(_SBus);
		ADD_MODULE(_PWMio);
		ADD_MODULE(_ProtocolBase);
#endif

#ifdef WITH_SCIENCE
#ifdef USE_MATHGL
		//		ADD_MODULE(_Solver);
		ADD_MODULE(_SolverBase);
		ADD_MODULE(_FourierSeries);
#endif
#endif

#ifdef WITH_SENSOR
		ADD_MODULE(_BenewakeTF);
		ADD_MODULE(_TOFsense);
		ADD_MODULE(_LeddarVu);
#ifdef USE_LIVOX
		ADD_MODULE(_Livox);
		ADD_MODULE(LivoxLidar);
#endif
#endif

#ifdef WITH_SLAM
#ifdef USE_OPENCV
#ifdef USE_ORB_SLAM
		ADD_MODULE(_ORB_SLAM);
#endif
#endif
#endif

#ifdef WITH_STATE
		ADD_MODULE(_StateControl);
#endif

#ifdef WITH_UI
#ifdef USE_OPENCV
		ADD_MODULE(_WindowCV);
		ADD_MODULE(_GstOutput);
#endif
#ifdef USE_GUI
		ADD_MODULE(_WindowGUI);
#endif
#endif

#ifdef WITH_UNIVERSE
		ADD_MODULE(_Object);
		ADD_MODULE(_Universe);
#endif

#ifdef WITH_VISION
#ifdef USE_OPENCV
		ADD_MODULE(_Camera);
		ADD_MODULE(_Contrast);
		ADD_MODULE(_DepthProj);
		ADD_MODULE(_Crop);
		ADD_MODULE(_Depth2Gray);
		ADD_MODULE(_Erode);
		ADD_MODULE(_GPhoto);
		ADD_MODULE(_GStreamer);
		ADD_MODULE(_Grayscale);
		ADD_MODULE(_HistEqualize);
		ADD_MODULE(_Invert);
		ADD_MODULE(_InRange);
		ADD_MODULE(_ImgFile);
		ADD_MODULE(_Mask);
		ADD_MODULE(_Morphology);
		ADD_MODULE(_Raspivid);
		ADD_MODULE(_Resize);
		ADD_MODULE(_Remap);
		ADD_MODULE(_Rotate);
		ADD_MODULE(_SharedMemImg);
		ADD_MODULE(_Threshold);
		ADD_MODULE(_VideoFile);
#ifdef USE_CUDA
		ADD_MODULE(_DenseFlow);
#endif
#ifdef USE_MYNTEYE
		ADD_MODULE(_Mynteye);
#endif
#ifdef USE_REALSENSE
		ADD_MODULE(_RealSense);
		ADD_MODULE(_DepthShow);
#endif
#ifdef USE_PYLON
		ADD_MODULE(_Pylon);
#endif
#endif	//opencv
#ifdef USE_VZENSE
		ADD_MODULE(_Vzense);
#endif
#endif	//vision

		return NULL;
	}

	template <typename T>
	BASE *Module::createInst(Kiss *pKiss)
	{
		IF_N(!pKiss);

		T *pInst = new T();
		return pInst;
	}

}
