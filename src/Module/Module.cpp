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
		NULL__(pK, nullptr);

		ADD_MODULE(SharedMem);
		ADD_MODULE(Destimator);
		ADD_MODULE(_Console);

// modules

#ifdef WITH_3D
		ADD_MODULE(_MeshStream);
		ADD_MODULE(_PCstream);
		ADD_MODULE(_PCframe);
		ADD_MODULE(_PCgrid);
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
#ifdef USE_GUI
		ADD_MODULE(_GeometryViewer);
#endif
#endif

#ifdef WITH_ACTUATOR
		ADD_MODULE(_ArduServo);
		ADD_MODULE(_MultiActuatorsBase);
		ADD_MODULE(_OrientalMotor);
		ADD_MODULE(_HYMCU_RS485);
		ADD_MODULE(_ZLAC8015);
		ADD_MODULE(_ZLAC8015D);
		ADD_MODULE(_ZDmotor);
		ADD_MODULE(_DDSM);
#ifdef USE_FEETECH
		ADD_MODULE(_Feetech);
#endif
#ifdef USE_XARM
		ADD_MODULE(_xArm);
#endif
#endif

#ifdef WITH_APMAVLINK
		ADD_MODULE(_APmavlink_rcChannel);
		ADD_MODULE(_APmavlink_base);
		ADD_MODULE(_APmavlink_httpJson);
		ADD_MODULE(_APmavlink_mav2json);
		ADD_MODULE(_APmavlink_mission);
		ADD_MODULE(_APmavlink_move);
		ADD_MODULE(_APmavlink_relay);
		ADD_MODULE(_APmavlink_servo);
		ADD_MODULE(_APmavlink_drive);
#ifdef WITH_SENSOR
		ADD_MODULE(_APmavlink_distLidar);
#endif
#ifdef WITH_SWARM
		ADD_MODULE(_APmavlink_swarm);
#endif
#ifdef USE_OPENCV
		ADD_MODULE(_APmavlink_avoid);
		ADD_MODULE(_APmavlink_depthVision);
		ADD_MODULE(_APmavlink_video);
		ADD_MODULE(_APmavlink_videoStream);
		ADD_MODULE(_APmavlink_photo);
#ifdef USE_OPENCV_CONTRIB
		ADD_MODULE(_APmavlink_follow);
		ADD_MODULE(_APmavlink_land);
		ADD_MODULE(_APmavlink_landingTarget);
#endif // opencv_contrib
#endif // opencv
#ifdef WITH_NAVIGATION
		ADD_MODULE(_APmavlink_visionEstimate);
#ifdef USE_REALSENSE
		ADD_MODULE(_APmavlink_GPS);
#endif
#endif // nav
#endif // apMavlink

#ifdef WITH_AUTOPILOT_DRIVE
		ADD_MODULE(_Drive);
#endif

#ifdef WITH_COMPUTE
#ifdef USE_OPENCL
		ADD_MODULE(clBase);
#endif
#endif

#ifdef WITH_CONTROL
		ADD_MODULE(PID);
#endif

#ifdef WITH_FILE
#ifdef USE_OPENCV
		ADD_MODULE(_PhotoTake);
		ADD_MODULE(_GPhotoTake);
		ADD_MODULE(_GDimgUploader);
		ADD_MODULE(_BBoxCutOut);
		ADD_MODULE(_CutOut);
#endif
#endif

#ifdef WITH_DETECTOR
#ifdef USE_OPENCV
		ADD_MODULE(_DNNclassifier);
		ADD_MODULE(_YOLOv8);
		ADD_MODULE(_YOLOv3);
		ADD_MODULE(_DNNtext);
		ADD_MODULE(_DepthSegment);
		ADD_MODULE(_IRLock);
		ADD_MODULE(_OpenPose);
		ADD_MODULE(_HandKey);
		ADD_MODULE(_Lane);
		ADD_MODULE(_Line);
		ADD_MODULE(_SSD);
		ADD_MODULE(_Contour);
#ifdef USE_OPENCV_CONTRIB
		ADD_MODULE(_ArUco);
		ADD_MODULE(_MotionDetector);
		ADD_MODULE(_SingleTracker);
#endif
#ifdef USE_CUDA
		ADD_MODULE(_Cascade);
#endif
#ifdef USE_CHILITAGS
		ADD_MODULE(_Chilitags);
#endif
#endif //USE_OPENCV
#endif

#ifdef WITH_DNN
#ifdef USE_OPENCV
#ifdef USE_JETSON_INFERENCE
		ADD_MODULE(_DetectNet);
#endif
#ifdef USE_TF-LITE
		ADD_MODULE(_TFmobileNet);
#endif
#endif
#endif

#ifdef WITH_FILTER
#endif

#ifdef WITH_IO
		ADD_MODULE(_ADIO_EBYTE);
		ADD_MODULE(_SerialPort);
		ADD_MODULE(_TCPserver);
		ADD_MODULE(_TCPclient);
		ADD_MODULE(_UDP);
#ifdef USE_WSSERVER
		ADD_MODULE(_WebSocket);
		ADD_MODULE(_WebSocketServer);
#endif
#endif

#ifdef WITH_NAVIGATION
		ADD_MODULE(_RTCM3);
		ADD_MODULE(_GPS);
		ADD_MODULE(GeoGrid);
#ifdef USE_REALSENSE
		ADD_MODULE(_RStracking);
#endif
#endif

#ifdef WITH_NET
		ADD_MODULE(_Uploader);
#endif

#ifdef WITH_PROTOCOL
		ADD_MODULE(_Canbus);
		ADD_MODULE(_JSONbase);
		ADD_MODULE(_Mavlink);
		ADD_MODULE(_Modbus);
		ADD_MODULE(_SBus);
		ADD_MODULE(_PWMio);
		ADD_MODULE(_ProtocolBase);
		ADD_MODULE(_Xbee);
#endif

#ifdef WITH_ROS
		ADD_MODULE(_ROS_fastLio);
#endif

#ifdef WITH_SCIENCE
		ADD_MODULE(_IsingSolver);
#ifdef USE_MATHGL
		//		ADD_MODULE(_Solver);
//		ADD_MODULE(_SolverBase);
//		ADD_MODULE(_FourierSeries);
#endif
#endif

#ifdef WITH_SENSOR
		ADD_MODULE(_BenewakeTF);
		ADD_MODULE(_TOFsense);
		ADD_MODULE(_LeddarVu);
#ifdef WITH_3D && USE_OPEN3D
		ADD_MODULE(_Livox2);
#endif // 3D
#endif // sensor

#ifdef WITH_SLAM && WITH_NAVIGATION
#ifdef USE_OPENCV
		ADD_MODULE(_SLAMbase);
#endif
#endif

#ifdef WITH_STATE
		ADD_MODULE(_StateControl);
#endif

#ifdef WITH_SWARM
		ADD_MODULE(_SwarmBase);
		ADD_MODULE(_SwarmCtrl);
		ADD_MODULE(_SwarmCtrlUI);
		ADD_MODULE(_SwarmSearch);
#endif

#ifdef WITH_UI
#ifdef USE_OPENCV
		ADD_MODULE(_GstOutput);
		ADD_MODULE(_WindowCV);
#endif
#ifdef USE_GUI
#endif
#endif

#ifdef WITH_UNIVERSE
		ADD_MODULE(_Object);
		ADD_MODULE(_Universe);
#endif

#ifdef WITH_VISION
#ifdef USE_ORBBEC
		ADD_MODULE(_Orbbec);
#endif
#ifdef USE_REALSENSE
		ADD_MODULE(_RealSense);
#endif
#ifdef USE_VZENSE
		ADD_MODULE(_Vzense);
#endif
#ifdef USE_XDYNAMICS
		ADD_MODULE(_XDynamics);
#endif
#ifdef USE_OPENCV
		ADD_MODULE(_Camera);
		ADD_MODULE(_Contrast);
		ADD_MODULE(_DepthProj);
		ADD_MODULE(_Crop);
		ADD_MODULE(_Depth2Gray);
		ADD_MODULE(_DepthShow);
		ADD_MODULE(_Erode);
		ADD_MODULE(_GPhoto);
		ADD_MODULE(_GStreamer);
		ADD_MODULE(_ColorConvert);
		ADD_MODULE(_HistEqualize);
		ADD_MODULE(_Invert);
		ADD_MODULE(_InRange);
		ADD_MODULE(_ImgFile);
		ADD_MODULE(_Mask);
		ADD_MODULE(_Morphology);
		ADD_MODULE(_Resize);
		ADD_MODULE(_Remap);
		ADD_MODULE(_Rotate);
		ADD_MODULE(_SharedMemImg);
		ADD_MODULE(_Threshold);
		ADD_MODULE(_VideoFile);
#ifdef USE_INFIRAY
		ADD_MODULE(_InfiRay);
#endif
#ifdef USE_CUDA
		ADD_MODULE(_DenseFlow);
#endif
#endif	//opencv
#endif	//vision

// Test modules

#ifdef WITH_TEST

#ifdef WITH_IO
#ifdef USE_WSSERVER
		ADD_MODULE(_TestWebSocket);
#endif
#endif

#ifdef WITH_PROTOCOL
		ADD_MODULE(_TestJSON);
#endif

#endif // test


		return NULL;
	}

	template <typename T>
	BASE *Module::createInst(Kiss *pKiss)
	{
		NULL__(pKiss, nullptr);

		T *pInst = new T();
		return pInst;
	}

}
