#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "ObjectDetector.h"
#include "CameraVision.h"
#include "AutoPilot.h"

//test
AutoPilot* g_pAP;
CameraVision* g_pCV;
ObjectDetector* g_pDetector;
MavlinkInterface* g_pMavlink;
cv::UMat g_frame;
cv::UMat g_upwardFrame;
cv::UMat g_displayMat;
fVector3 g_targetPosExt;

int g_key;
bool g_bRun;
bool g_bTracking;

VideoCapture g_externalCam;
VideoCapture g_upwardCam;

bool g_bUpwardCam;

JSONParser g_Json;

struct PID_UI
{
	int m_P;
	int m_I;
	int m_D;
	int m_Z;
};

PID_UI g_rollFar;
PID_UI g_rollNear;
PID_UI g_altFar;
PID_UI g_altNear;
PID_UI g_pitchFar;
PID_UI g_pitchNear;
PID_UI g_yawFar;
PID_UI g_yawNear;
int g_dT;

void onMouse(int event, int x, int y, int flags, void* userdata);
void createConfigWindow(void);
void onTrackbar(int, void*);
void displayInfo(void);
void handleKey(int key);

int main(int argc, char* argv[])
{
	// Initialize Google's logging library.
	google::InitGoogleLogging(argv[0]);
	namedWindow(APP_NAME);




/*
	if(m_jsonParser.init()==false)
		{
			m_logger.addLog(0,LOG_ERROR,"U_JSONParser.init()");
			return false;
		}

		if(!m_fileIO.open(CONFIG_FILENAME))
		{
			m_logger.addLog(0,LOG_ERROR,"U_FileIO.open()");
			return false;
		}
		string config = m_fileIO.getContent();

		if(!m_jsonParser.parse(config.c_str()))
		{
			m_logger.addLog(0,LOG_ERROR,"U_JSONParser.parse()");
			return false;
		}

		if(!m_jsonParser.getVal("vehicleName",&m_name))
		{
			m_logger.addLog(0,LOG_ERROR,"U_JSONParser.getVal():vehicleName");
			return false;
		}
*/









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
		LOG(ERROR) << "Cannot open serial port, Working in CV mode only";
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
		LOG(ERROR) << "Cannot open External camera\n";
		return false;
	}
	g_externalCam.set(CV_CAP_PROP_FRAME_WIDTH, CAM_WIDTH);
	g_externalCam.set(CV_CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT);

	//Open the upward camera
	g_upwardCam.open(CAM_UPWARD_ID); // + CV_CAP_DSHOW);//CV_CAP_MSMF
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
		g_pDetector->detect(g_pCV->getMat()->getMat(ACCESS_READ));

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

void handleKey(int key)
{
	switch (key)
	{
	case 'a':
		//Going forward
		if (g_targetPosExt.m_z > Z_FAR_LIM)
		{
			g_targetPosExt.m_z -= 5;
			g_pAP->setTargetPosCV(g_targetPosExt);
		}
		break;
	case 'z':
		//Going backward
		if (g_targetPosExt.m_z < Z_NEAR_LIM)
		{
			g_targetPosExt.m_z += 5;
			g_pAP->setTargetPosCV(g_targetPosExt);
		}
		break;
	case 'c':
		//Config window
		createConfigWindow();
		break;
	case 'm':
		g_pAP->m_hostSystem.m_mode = OPE_MANUAL;
		break;
	case 27:
		g_bRun = false;	//ESC
		break;
	default:
		break;
	}
}

void onMouse(int event, int x, int y, int flags, void* userdata)
{
	if (x > CAM_WIDTH || y > CAM_HEIGHT)
		return;

	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		g_bTracking = true;
		g_targetPosExt = g_pAP->getTargetPosCV();
//		g_targetPosExt = fVector3{ x, y, g_targetPosExt.m_z };
		g_targetPosExt.m_x = x;
		g_targetPosExt.m_y = y;
//		g_targetPosExt.m_z = g_targetPosExt.m_z;

		g_pAP->setTargetPosCV(g_targetPosExt);
		break;
	case EVENT_LBUTTONUP:
		g_bTracking = false;
		break;
	case EVENT_MOUSEMOVE:
		if (!g_bTracking)
			return;
		g_targetPosExt = g_pAP->getTargetPosCV();
//		g_targetPosExt = fVector3{ x, y, g_targetPosExt.m_z };
		g_targetPosExt.m_x = x;
		g_targetPosExt.m_y = y;
//		g_targetPosExt.m_z = g_targetPosExt.m_z;
		g_pAP->setTargetPosCV(g_targetPosExt);
		break;
	case EVENT_RBUTTONDOWN:
		//Re-locate the ROI for object recognition
		fVector3 v;
		v.m_x = x;
		v.m_y = y;
		v.m_z = 10;
//		g_pCV->setObjROI(fVector3{ x, y, 10 });
		g_pCV->setObjROI(v);
		break;
	default:
		break;
	}
}

void onTrackbar(int, void*)
{
	//Get current value
	PID_SETTING pid;

	g_pAP->setDelayTime(
			-DT_LIM * (PID_UI_LIM_HALF - (double) g_dT) / PID_UI_LIM_HALF);

	pid.m_P = -PID_P_LIM
			* (PID_UI_LIM_HALF - (double) g_rollFar.m_P)/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM
			* (PID_UI_LIM_HALF - (double) g_rollFar.m_I)/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM
			* (PID_UI_LIM_HALF - (double) g_rollFar.m_D)/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM
			* (PID_UI_LIM_HALF - (double) g_rollFar.m_Z)/ PID_UI_LIM_HALF;
	g_pAP->setRollFarPID(pid);

	pid.m_P = -PID_P_LIM
			* (PID_UI_LIM_HALF - (double) g_rollNear.m_P)/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM
			* (PID_UI_LIM_HALF - (double) g_rollNear.m_I)/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM
			* (PID_UI_LIM_HALF - (double) g_rollNear.m_D)/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM
			* (PID_UI_LIM_HALF - (double) g_rollNear.m_Z)/ PID_UI_LIM_HALF;
	g_pAP->setRollNearPID(pid);

	pid.m_P = -PID_P_LIM
			* (PID_UI_LIM_HALF - (double) g_altFar.m_P)/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM
			* (PID_UI_LIM_HALF - (double) g_altFar.m_I)/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM
			* (PID_UI_LIM_HALF - (double) g_altFar.m_D)/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM
			* (PID_UI_LIM_HALF - (double) g_altFar.m_Z)/ PID_UI_LIM_HALF;
	g_pAP->setAltFarPID(pid);

	pid.m_P = -PID_P_LIM
			* (PID_UI_LIM_HALF - (double) g_altNear.m_P)/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM
			* (PID_UI_LIM_HALF - (double) g_altNear.m_I)/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM
			* (PID_UI_LIM_HALF - (double) g_altNear.m_D)/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM
			* (PID_UI_LIM_HALF - (double) g_altNear.m_Z)/ PID_UI_LIM_HALF;
	g_pAP->setAltNearPID(pid);

	pid.m_P = -PID_P_LIM
			* (PID_UI_LIM_HALF - (double) g_pitchFar.m_P)/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM
			* (PID_UI_LIM_HALF - (double) g_pitchFar.m_I)/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM
			* (PID_UI_LIM_HALF - (double) g_pitchFar.m_D)/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM
			* (PID_UI_LIM_HALF - (double) g_pitchFar.m_Z)/ PID_UI_LIM_HALF;
	g_pAP->setPitchFarPID(pid);

	pid.m_P = -PID_P_LIM
			* (PID_UI_LIM_HALF - (double) g_pitchNear.m_P)/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM
			* (PID_UI_LIM_HALF - (double) g_pitchNear.m_I)/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM
			* (PID_UI_LIM_HALF - (double) g_pitchNear.m_D)/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM
			* (PID_UI_LIM_HALF - (double) g_pitchNear.m_Z)/ PID_UI_LIM_HALF;
	g_pAP->setPitchNearPID(pid);

	pid.m_P = -PID_YAW_P_LIM
			* (PID_UI_LIM_HALF - (double) g_yawFar.m_P)/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM
			* (PID_UI_LIM_HALF - (double) g_yawFar.m_I)/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM
			* (PID_UI_LIM_HALF - (double) g_yawFar.m_D)/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM
			* (PID_UI_LIM_HALF - (double) g_yawFar.m_Z)/ PID_UI_LIM_HALF;
	g_pAP->setYawFarPID(pid);

	pid.m_P = -PID_YAW_P_LIM
			* (PID_UI_LIM_HALF - (double) g_yawNear.m_P)/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM
			* (PID_UI_LIM_HALF - (double) g_yawNear.m_I)/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM
			* (PID_UI_LIM_HALF - (double) g_yawNear.m_D)/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM
			* (PID_UI_LIM_HALF - (double) g_yawNear.m_Z)/ PID_UI_LIM_HALF;
	g_pAP->setYawNearPID(pid);

}

void createConfigWindow(void)
{
	namedWindow(CONFIG_WINDOW);

	//Get current value
	PID_SETTING pid;

	g_dT = (g_pAP->getDelayTime() / DT_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getRollFarPID();
	g_rollFar.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollFar.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollFar.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollFar.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getRollNearPID();
	g_rollNear.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollNear.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollNear.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollNear.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getAltFarPID();
	g_altFar.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altFar.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altFar.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altFar.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getAltNearPID();
	g_altNear.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altNear.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altNear.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altNear.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getPitchFarPID();
	g_pitchFar.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchFar.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchFar.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchFar.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getPitchNearPID();
	g_pitchNear.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchNear.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchNear.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchNear.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getYawFarPID();
	g_yawFar.m_P =
			(pid.m_P / PID_YAW_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawFar.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawFar.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawFar.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getYawNearPID();
	g_yawNear.m_P = (pid.m_P / PID_YAW_P_LIM) * PID_UI_LIM_HALF
			+ PID_UI_LIM_HALF;
	g_yawNear.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawNear.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawNear.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	//Trackbar for configuration
	createTrackbar("dT", CONFIG_WINDOW, &g_dT, PID_UI_LIM, onTrackbar);

	//Roll
	createTrackbar("rollFar_P", CONFIG_WINDOW, &g_rollFar.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollFar_I", CONFIG_WINDOW, &g_rollFar.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollFar_D", CONFIG_WINDOW, &g_rollFar.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollFar_Z", CONFIG_WINDOW, &g_rollFar.m_Z, PID_UI_LIM,
			onTrackbar);

	createTrackbar("rollNear_P", CONFIG_WINDOW, &g_rollNear.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollNear_I", CONFIG_WINDOW, &g_rollNear.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollNear_D", CONFIG_WINDOW, &g_rollNear.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollNear_Z", CONFIG_WINDOW, &g_rollNear.m_Z, PID_UI_LIM,
			onTrackbar);

	//Alt
	createTrackbar("altFar_P", CONFIG_WINDOW, &g_altFar.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altFar_I", CONFIG_WINDOW, &g_altFar.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altFar_D", CONFIG_WINDOW, &g_altFar.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altFar_Z", CONFIG_WINDOW, &g_altFar.m_Z, PID_UI_LIM,
			onTrackbar);

	createTrackbar("altNear_P", CONFIG_WINDOW, &g_altNear.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altNear_I", CONFIG_WINDOW, &g_altNear.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altNear_D", CONFIG_WINDOW, &g_altNear.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altNear_Z", CONFIG_WINDOW, &g_altNear.m_Z, PID_UI_LIM,
			onTrackbar);

	//Pitch
	createTrackbar("pitchFar_P", CONFIG_WINDOW, &g_pitchFar.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchFar_I", CONFIG_WINDOW, &g_pitchFar.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchFar_D", CONFIG_WINDOW, &g_pitchFar.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchFar_Z", CONFIG_WINDOW, &g_pitchFar.m_Z, PID_UI_LIM,
			onTrackbar);

	createTrackbar("pitchNear_P", CONFIG_WINDOW, &g_pitchNear.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchNear_I", CONFIG_WINDOW, &g_pitchNear.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchNear_D", CONFIG_WINDOW, &g_pitchNear.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchNear_Z", CONFIG_WINDOW, &g_pitchNear.m_Z, PID_UI_LIM,
			onTrackbar);

	//Yaw
	createTrackbar("yawFar_P", CONFIG_WINDOW, &g_yawFar.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawFar_I", CONFIG_WINDOW, &g_yawFar.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawFar_D", CONFIG_WINDOW, &g_yawFar.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawFar_Z", CONFIG_WINDOW, &g_yawFar.m_Z, PID_UI_LIM,
			onTrackbar);

	createTrackbar("yawNear_P", CONFIG_WINDOW, &g_yawNear.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawNear_I", CONFIG_WINDOW, &g_yawNear.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawNear_D", CONFIG_WINDOW, &g_yawNear.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawNear_Z", CONFIG_WINDOW, &g_yawNear.m_Z, PID_UI_LIM,
			onTrackbar);
}

void displayInfo(void)
{
	char strBuf[512];
	std::string strInfo;
	PID_SETTING pid;
	fVector3 pid3;
	fVector3 vPos;
	fVector3 vAtt;
	cv::Rect roi;
	cv::UMat* pExtMat;
	int i;
	int startPosH = 25;
	int startPosV = 725;
	int lineHeight = 20;
	int* pPWM;
	CONTROL_AXIS* pControl;
	i = 0;

	//	m_gBalloonyness.download(matBalloonyness);
	//	imshow("balloonyness", matBalloonyness);

	if (!g_pCV->getObjPosition(&vPos))
	{
//		vPos = fVector3{0, 0, 0};
		vPos.m_x = 0;
		vPos.m_y = 0;
		vPos.m_z = 0;
	}
	if (!g_pCV->getObjAttitude(&vAtt))
	{
//		vAtt = fVector3{ 0, 0,0 };
		vAtt.m_x = 0;
		vAtt.m_y = 0;
		vAtt.m_z = 0;
	}
	g_targetPosExt = g_pAP->getTargetPosCV();

	g_displayMat = Scalar(0);

	//External Camera Output
	pExtMat = g_pCV->getMat();
	roi = cv::Rect(cv::Point(0, 0), pExtMat->size());
	pExtMat->copyTo(g_displayMat(roi));

	//Vehicle position
	cv::circle(g_displayMat, Point(vPos.m_x, vPos.m_y), vPos.m_z * 0.5,
			Scalar(0, 255, 0), 3);

	//Target position
	cv::circle(g_displayMat, Point(g_targetPosExt.m_x, g_targetPosExt.m_y),
			g_targetPosExt.m_z * 0.5, Scalar(0, 255, 255), 3);

	//Title
	cv::putText(g_displayMat, "External CAM", Point(startPosH, 25),
			FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);

	//Information
	sprintf(strBuf, "Pos: X=%.2f, Y=%.2f, Z=%.2f", vPos.m_x, vPos.m_y,
			vPos.m_z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	sprintf(strBuf, "Att: R=%.2f, P=%.2f, Y=%.2f", vAtt.m_x, vAtt.m_y,
			vAtt.m_z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	sprintf(strBuf, "Target Pos: X=%.2f, Y=%.2f, Z=%.2f, dT=%.2f",
			g_targetPosExt.m_x, g_targetPosExt.m_y, g_targetPosExt.m_z,
			g_pAP->getDelayTime());
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	i++;

	pid3 = g_pAP->getRollPID();
	sprintf(strBuf, "Roll: P=%.2f, I=%.2f, D=%.2f", pid3.m_x, pid3.m_y,
			pid3.m_z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid3 = g_pAP->getAltPID();
	sprintf(strBuf, "Alt: P=%.2f, I=%.2f, D=%.2f", pid3.m_x, pid3.m_y,
			pid3.m_z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid3 = g_pAP->getPitchPID();
	sprintf(strBuf, "Pitch: P=%.2f, I=%.2f, D=%.2f", pid3.m_x, pid3.m_y,
			pid3.m_z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid3 = g_pAP->getYawPID();
	sprintf(strBuf, "Yaw: P=%.2f, I=%.2f, D=%.2f", pid3.m_x, pid3.m_y,
			pid3.m_z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	i++;

	pid = g_pAP->getRollFarPID();
	sprintf(strBuf, "Roll_Far: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->getRollNearPID();
	sprintf(strBuf, "Roll_Near: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->getAltFarPID();
	sprintf(strBuf, "Alt_Far: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P, pid.m_I,
			pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->getAltNearPID();
	sprintf(strBuf, "Alt_Near: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->getPitchFarPID();
	sprintf(strBuf, "Pitch_Far: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->getPitchNearPID();
	sprintf(strBuf, "Pitch_Near: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->getYawFarPID();
	sprintf(strBuf, "Yaw_Far: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P, pid.m_I,
			pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->getYawNearPID();
	sprintf(strBuf, "Yaw_Near: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	i = 0;
	startPosH = 600;

	pPWM = g_pAP->getPWMOutput();
	sprintf(strBuf, "PWM: Roll=%d, Pitch=%d, Alt=%d, Yaw=%d", pPWM[0], pPWM[1],
			pPWM[2], pPWM[3]);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pControl = g_pAP->getRollAxis();
	sprintf(strBuf, "ROLL_I_PWM: %.2f", pControl->m_cvErrInteg * pControl->m_I);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pControl = g_pAP->getPitchAxis();
	sprintf(strBuf, "PITCH_I_PWM: %.2f",
			pControl->m_cvErrInteg * pControl->m_I);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
/*
	for (size_t j = 0; j < g_predictions.size(); ++j)
	{
		Prediction p = g_predictions[j];
//		std::cout << std::fixed << std::setprecision(4) << p.second << " - \"" << p.first << "\"" << std::endl;
		cv::putText(g_displayMat, String(p.first),
				Point(startPosH, startPosV + lineHeight * (++i)),
				FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	}
*/
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
