#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "ObjectDetector.h"
#include "CameraVision.h"
#include "AutoPilot.h"

#include "demo.h"

int main(int argc, char* argv[])
{
	// Initialize Google's logging library.
	google::InitGoogleLogging(argv[0]);

	namedWindow(APP_NAME);


	if (!g_file.open(argv[1]))//"/Users/yankai/Documents/workspace/LAB/src/config.txt"))
	{
		LOG(FATAL)<<"FileIO.open()";
		return 1;
	}
	string config = g_file.getContent();

	if (!g_Json.parse(config.c_str()))
	{
		LOG(FATAL)<<"JSONParser.parse()";
		return 1;
	}

	if (!g_Json.getVal("serialPort", &g_serialPort))
	{
		LOG(FATAL)<<"JSONParser.getVal():serialPort";
		return 1;
	}

	g_bRun = true;
	g_bTracking = false;

	//Connect to Mavlink
#ifdef PLATFORM_WIN
	char pSportName[] = "\\\\.\\COM11";
#else
	char pSportName[] = "/dev/ttyACM0";
#endif
	g_pMavlink = new MavlinkInterface();

	if (g_pMavlink->open(pSportName) != true)
	{
		LOG(ERROR)<< "Cannot open serial port, Working in CV mode only";
	}
	else
	{
		LOG(ERROR) << "Serial port connected";
	}

	printf("OpenCV Drone Control\n");
	printf("Optimized code: %d\n", useOptimized());
	printf("CUDA devices: %d\n", cuda::getCudaEnabledDeviceCount());
	printf("Current CUDA device: %d\n", cuda::getDevice());

	if (ocl::haveOpenCL())
	{
		printf("OpenCL is found\n");
		ocl::setUseOpenCL(true);
		if (ocl::useOpenCL())
		{
			printf("Using OpenCL\n");
		}
	}
	else
	{
		printf("OpenCL not found\n");
	}

	//Open external camera
	g_externalCam.open(CAM_EXTERNAL_ID); // + CV_CAP_DSHOW);//CV_CAP_MSMF
	if (!g_externalCam.isOpened())
	{
		LOG(ERROR)<< "Cannot open External camera\n";
		return false;
	}
	g_externalCam.set(CV_CAP_PROP_FRAME_WIDTH, CAM_WIDTH);
	g_externalCam.set(CV_CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT);

	//Open the upward camera
//	g_upwardCam.open(CAM_UPWARD_ID); // + CV_CAP_DSHOW);//CV_CAP_MSMF
	g_bUpwardCam = true;
	g_bUpwardCam = false;

	/*	if (!g_upwardCam.isOpened())
	 {
	 g_pLogger->print("Cannot open Upward camera\n");
	 g_bUpwardCam = false;
	 }
	 else
	 {
	 g_upwardCam.set(CV_CAP_PROP_FRAME_WIDTH, CAM_WIDTH);
	 g_upwardCam.set(CV_CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT);
	 }
	 */
	//Initial target position
	g_targetPosExt.m_x = CAM_WIDTH * 0.5;
	g_targetPosExt.m_y = CAM_HEIGHT * 0.5;
	g_targetPosExt.m_z = (Z_NEAR_LIM + Z_FAR_LIM) * 0.2;

	//Init CV
	g_pCV = new CameraVision();
	g_pCV->init();

#ifdef USE_MARKER_FLOW
	g_pCV->initMarkerFlow();
#endif
#ifdef USE_FEATURE_FLOW
	g_pCV->initFeatureFlow();
#endif
#ifdef USE_OPTICAL_FLOW
	g_pCV->initOpticalFlow();
#endif

	//Init Autopilot
	g_pAP = new AutoPilot();
	g_pAP->init();
	g_pAP->setCameraVision(g_pCV);
	g_pAP->setMavlink(g_pMavlink);
	g_pAP->setTargetPosCV(g_targetPosExt);
	g_pAP->setDelayTime(DELAY_TIME);

	//Mat for display output
	g_displayMat = UMat(CAM_HEIGHT * 2, CAM_WIDTH * 2, CV_8UC3, Scalar(0));

	//set the callback function for any mouse event
	setMouseCallback(APP_NAME, onMouse, NULL);

	//Init Detector
	g_pDetector = new ObjectDetector();
	g_pDetector->init();

	while (g_bRun)
	{
		while (!g_externalCam.read(g_frame))
			;
		g_pCV->updateFrame(&g_frame);

#ifdef USE_MARKER_FLOW
		g_pCV->updateMarkerFlow();
//		g_pAP->markerLock();
#endif

#ifdef USE_OPTICAL_FLOW
//		g_pCV->updateFeatureFlow();
		g_pCV->updateOpticalFlow();
		g_pAP->flowLock();
#endif

//		g_predictions = classifier.Classify(g_pCV->getMat()->getMat(ACCESS_READ));

		/* Print the top N predictions. */
		/*		for (size_t i = 0; i < g_predictions.size(); ++i) {
		 Prediction p = g_predictions[i];
		 std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
		 << p.first << "\"" << std::endl;
		 }
		 */
		g_numObj = g_pDetector->detect(g_pCV->getMat()->getMat(ACCESS_READ),
				&g_pObj);

		if (g_bUpwardCam)
		{
			while (!g_upwardCam.read(g_upwardFrame))
				;
			cv::Rect roi(cv::Point(CAM_WIDTH, 0), g_upwardFrame.size());
			g_upwardFrame.copyTo(g_displayMat(roi));
		}

		displayInfo();
		imshow(APP_NAME, g_displayMat);

		//Handle key input
		g_key = waitKey(1);
		handleKey(g_key);
	}

	g_externalCam.release();
	if (g_bUpwardCam)
	{
		g_upwardCam.release();
	}
	g_pMavlink->close();
	delete g_pMavlink;
	delete g_pAP;
	delete g_pCV;

	return 0;

}

/*
 #include <opencv2/highgui/highgui.hpp>
 #include <opencv2/imgproc/imgproc.hpp>
 #include <iostream>
 using namespace cv;
 using namespace std;

 int main() {
 VideoCapture stream1(1);   //0 is the id of video device.0 if you have only one camera.

 if (!stream1.isOpened()) { //check if video device has been initialised
 cout << "cannot open camera";
 return 1;
 }

 namedWindow("cam");
 Mat cameraFrame;

 while(true)
 {
 while(!stream1.read(cameraFrame));
 imshow("cam", cameraFrame);
 if (waitKey(1) >= 0)
 {
 break;
 }
 }
 return 0;
 }
 */
