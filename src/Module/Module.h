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
#include "../Module/Kiss.h"

#include "../Arithmetic/Destimator.h"
#include "../IPC/SharedMem.h"
#include "../UI/_Console.h"

// modules

#ifdef WITH_3D
#include "../3D/Mesh/_MeshStream.h"
#include "../3D/PointCloud/_PCstream.h"
#include "../3D/PointCloud/_PCframe.h"
#include "../3D/PointCloud/_PCgrid.h"
#include "../3D/PointCloud/_PCfile.h"
#include "../3D/PointCloud/_RGBD2PCframe.h"
#include "../3D/PointCloud/_PCmerge.h"
#include "../3D/PointCloud/_PCsend.h"
#include "../3D/PointCloud/_PCrecv.h"
#include "../3D/PointCloud/PCfilter/_PCtransform.h"
#include "../3D/PointCloud/PCfilter/_PCcrop.h"
#include "../3D/PointCloud/PCfilter/_PCremove.h"
#include "../3D/PointCloud/PCfilter/_PCdownSample.h"
#include "../3D/PointCloud/PCregistration/_PCregistCol.h"
#include "../3D/PointCloud/PCregistration/_PCregistICP.h"
#include "../3D/PointCloud/PCregistration/_PCregistGlobal.h"
#ifdef USE_GUI
#include "../3D/_GeometryViewer.h"
#endif
#endif

#ifdef WITH_ACTUATOR
#include "../Actuator/_ArduServo.h"
#include "../Actuator/_MultiActuatorsBase.h"
#include "../Actuator/_OrientalMotor.h"
#include "../Actuator/_HYMCU_RS485.h"
#include "../Actuator/Motor/_ZLAC8015.h"
#include "../Actuator/Motor/_ZLAC8015D.h"
#include "../Actuator/Motor/_ZDmotor.h"
#include "../Actuator/Motor/_DDSM.h"
#ifdef USE_FEETECH
#include "../Actuator/_Feetech.h"
#endif
#ifdef USE_XARM
#include "../Actuator/Articulated/_xArm.h"
#endif
#endif

#ifdef WITH_APMAVLINK
#include "../Autopilot/APmavlink/_APmavlink_rcChannel.h"
#include "../Autopilot/APmavlink/_APmavlink_base.h"
#include "../Autopilot/APmavlink/_APmavlink_httpJson.h"
#include "../Autopilot/APmavlink/_APmavlink_mav2json.h"
#include "../Autopilot/APmavlink/_APmavlink_mission.h"
#include "../Autopilot/APmavlink/_APmavlink_move.h"
#include "../Autopilot/APmavlink/_APmavlink_relay.h"
#include "../Autopilot/APmavlink/_APmavlink_servo.h"
#include "../Autopilot/APmavlink/_APmavlink_drive.h"
#ifdef WITH_SENSOR
#include "../Autopilot/APmavlink/_APmavlink_distLidar.h"
#endif
#ifdef USE_OPENCV
#include "../Autopilot/APmavlink/_APmavlink_avoid.h"
#include "../Autopilot/APmavlink/_APmavlink_depthVision.h"
#include "../Autopilot/APmavlink/_APmavlink_video.h"
#include "../Autopilot/APmavlink/_APmavlink_videoStream.h"
#include "../Autopilot/APmavlink/_APmavlink_photo.h"
#ifdef USE_OPENCV_CONTRIB
#include "../Autopilot/APmavlink/_APmavlink_follow.h"
#include "../Autopilot/APmavlink/_APmavlink_land.h"
#include "../Autopilot/APmavlink/_APmavlink_landingTarget.h"
#endif // opencv_contrib
#endif // opencv
#ifdef WITH_SWARM
#include "../Autopilot/APmavlink/_APmavlink_swarm.h"
#endif
#ifdef WITH_NAVIGATION
#include "../Autopilot/APmavlink/_APmavlink_visionEstimate.h"
#ifdef USE_REALSENSE
#include "../Autopilot/APmavlink/_APmavlink_GPS.h"
#endif
#endif // nav
#endif // APmavlink

#ifdef WITH_AUTOPILOT_DRIVE
#include "../Autopilot/Drive/_Drive.h"
#endif

#ifdef WITH_COMPUTE
#ifdef USE_OPENCL
#include "../Compute/OpenCL/clBase.h"
#endif
#endif

#ifdef WITH_CONTROL
#include "../Control/PID.h"
#endif

#ifdef WITH_FILE
#ifdef USE_OPENCV
#include "../Data/Image/_PhotoTake.h"
#include "../Data/Image/_GPhotoTake.h"
#include "../Data/Image/_GDimgUploader.h"
#include "../Data/Image/_BBoxCutOut.h"
#include "../Data/Image/_CutOut.h"
#endif
#endif

#ifdef WITH_DETECTOR
#ifdef USE_OPENCV
#include "../Detector/_Lane.h"
#include "../Detector/_YOLOv3.h"
#include "../Detector/_YOLOv8.h"
#include "../Detector/_DNNclassifier.h"
#include "../Detector/_DNNtext.h"
#include "../Detector/_IRLock.h"
#include "../Detector/_Line.h"
#include "../Detector/_OpenPose.h"
#include "../Detector/_HandKey.h"
#include "../Detector/_SSD.h"
#include "../Detector/_Contour.h"
#ifdef USE_OPENCV_CONTRIB
#include "../Detector/_ArUco.h"
#include "../Detector/_MotionDetector.h"
#include "../Tracker/_SingleTracker.h"
#endif
#ifdef USE_CUDA
#include "../Detector/_Cascade.h"
#endif
#ifdef USE_CHILITAGS
#include "../Detector/_Chilitags.h"
#endif
#endif
#endif

#ifdef WITH_DNN
#ifdef USE_OPENCV
#ifdef USE_JETSON_INFERENCE
#include "../DNN/JetsonInference/_DetectNet.h"
#endif
#ifdef USE_TF - LITE
#include "../DNN/TensorFlowLite/_TFmobileNet.h"
#endif
#endif
#endif

#ifdef WITH_FILTER
#endif

#ifdef WITH_IO
#include "../IO/_ADIO_EBYTE.h"
#include "../IO/_File.h"
#include "../IO/_TCPserver.h"
#include "../IO/_TCPclient.h"
#include "../IO/_SerialPort.h"
#include "../IO/_UDP.h"
#ifdef USE_WSSERVER
#include "../IO/_WebSocket.h"
#include "../IO/_WebSocketServer.h"
#endif
#endif

#ifdef WITH_NAVIGATION
#include "../Navigation/_GPS.h"
#include "../Navigation/_RTCM3.h"
#include "../Navigation/GeoGrid.h"
#ifdef USE_REALSENSE
#include "../Navigation/_RStracking.h"
#endif
#endif

#ifdef WITH_NET
#include "../Net/_Uploader.h"
#endif

#ifdef WITH_PROTOCOL
#include "../Protocol/_USR_CANET.h"
#include "../Protocol/_JSONbase.h"
#include "../Protocol/_Mavlink.h"
#include "../Protocol/_PWMio.h"
#include "../Protocol/_ProtocolBase.h"
#include "../Protocol/_Modbus.h"
#include "../Protocol/_SBus.h"
#include "../Protocol/_Xbee.h"
#endif

#ifdef WITH_ROS
#include "../ROS/_ROS_fastLio.h"
#endif

#ifdef WITH_SCIENCE
#include "../Science/_IsingSolver.h"
#ifdef USE_MATHGL
// #include "../Science/_Solver.h"
//#include "../Science/_SolverBase.h"
//#include "../Science/_FourierSeries.h"
#endif
#endif

#ifdef WITH_SENSOR
#include "../Sensor/Distance/_LeddarVu.h"
#include "../Sensor/Distance/_TOFsense.h"
#include "../Sensor/Distance/_BenewakeTF.h"
#ifdef WITH_3D && USE_OPEN3D
#include "../Sensor/LiDAR/_Livox2.h"
#include "../Sensor/LiDAR/_RoboSenseAiry.h"
#endif // 3D
#endif // sensor

#ifdef WITH_SLAM && WITH_NAVIGATION
#ifdef USE_OPENCV
#include "../SLAM/_SLAMbase.h"
#endif
#endif

#ifdef WITH_STATE
#include "../State/_StateControl.h"
#endif

#ifdef WITH_SWARM
#include "../Swarm/_SwarmBase.h"
#include "../Swarm/_SwarmCtrl.h"
#include "../Swarm/_SwarmCtrlUI.h"
#include "../Swarm/_SwarmSearch.h"
#endif

#ifdef WITH_UI
#ifdef USE_OPENCV
#include "../UI/_GstOutput.h"
#include "../UI/_WindowCV.h"
#endif
#ifdef USE_GUI
#endif
#endif

#ifdef WITH_UNIVERSE
#include "../Universe/_Object.h"
#include "../Universe/_Universe.h"
#endif

#ifdef WITH_VISION
#ifdef USE_ORBBEC
#include "../Vision/RGBD/_Orbbec.h"
#endif
#ifdef USE_REALSENSE
#include "../Vision/RGBD/_RealSense.h"
#endif
#ifdef USE_SCEPTER_SDK
#include "../Vision/RGBD/_Scepter.h"
#endif
#ifdef USE_XDYNAMICS
#include "../Vision/RGBD/_XDynamics.h"
#endif
#ifdef USE_OPENCV
#include "../Vision/_Camera.h"
#include "../Vision/_UVC.h"
#include "../Vision/_VideoFile.h"
#include "../Vision/_ImgFile.h"
#include "../Vision/_GStreamer.h"
#include "../Vision/_SharedMemImg.h"
#include "../Vision/_GPhoto.h"
#include "../Vision/ImgFilter/_ColorConvert.h"
#include "../Vision/ImgFilter/_Contrast.h"
#include "../Vision/ImgFilter/_Crop.h"
#include "../Vision/RGBD/_D2G.h"
#include "../Vision/RGBD/_D2RGB.h"
#include "../Vision/ImgFilter/_Erode.h"
#include "../Vision/ImgFilter/_HistEqualize.h"
#include "../Vision/ImgFilter/_Invert.h"
#include "../Vision/ImgFilter/_InRange.h"
#include "../Vision/ImgFilter/_Morphology.h"
#include "../Vision/ImgFilter/_Mask.h"
#include "../Vision/ImgFilter/_Resize.h"
#include "../Vision/ImgFilter/_Remap.h"
#include "../Vision/ImgFilter/_Rotate.h"
#include "../Vision/ImgFilter/_Threshold.h"

#ifdef USE_CUDA
#include "../Vision/_DenseFlow.h"
#endif
#endif // opencv
#endif // vision

// Test modules

#ifdef WITH_TEST

#ifdef WITH_IO
#ifdef USE_WSSERVER
#include "../../test/IO/_TestWebSocket.h"
#endif
#endif

#ifdef WITH_PROTOCOL
#include "../../test/Protocol/_TestJSON.h"
#endif

#endif // test


#define ADD_MODULE(x)             \
	if (pK->getClass() == #x)     \
	{                             \
		return createInst<x>(pK); \
	}

namespace kai
{

	class Module
	{
	public:
		Module();
		virtual ~Module();
		BASE *createInstance(Kiss *pK);

	private:
		template <typename T>
		BASE *createInst(Kiss *pKiss);
	};

}

#endif
