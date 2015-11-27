#include "demo.h"

int main(int argc, char* argv[])
{
	//Init Logger
	google::InitGoogleLogging(argv[0]);
	printEnvironment();

	//Load config
	LOG(INFO)<<"Using config file: "<<argv[1];
	CHECK_FATAL(g_file.open(argv[1]));
	string config = g_file.getContent();
	CHECK_FATAL(g_Json.parse(config.c_str()));

	//Connect to Mavlink
	g_pMavlink = new MavlinkInterface();
	CHECK_FATAL(g_pMavlink->setup(&g_Json, "FC"));
	CHECK_INFO(g_pMavlink->open());

	//Init Camera
	g_pCamFront = new CamStream();
	CHECK_FATAL(g_pCamFront->setup(&g_Json, "FRONTL"));
	CHECK_FATAL(g_pCamFront->init());
	g_pCamFront->m_bGray = true;
	g_pCamFront->m_bHSV = true;
	g_pCamFront->m_bDenseFlow = true;

	//Init Object Detector
	g_pOD = new ObjectDetector();
	g_pOD->init(&g_Json);
	g_pOD->setCamStream(g_pCamFront);

	//Init Fast Detector
	g_pFD = new FastDetector();
	g_pFD->init(&g_Json);
	g_pFD->setCamStream(g_pCamFront);

	//Init Autopilot
	g_pAP = new AutoPilot();
	CHECK_FATAL(g_pAP->setup(&g_Json, ""));
	g_pAP->init();
	g_pAP->setCamStream(g_pCamFront, CAM_FRONT);
	g_pAP->m_pOD = g_pOD;
	g_pAP->m_pFD = g_pFD;
//	g_pMD = g_pAP->m_pCamStream[CAM_FRONT].m_pCam->m_pMarkerDetect;

	//Main window
	g_pShow = new CamFrame();

	//Init UI Monitor
	g_pUIMonitor = new UIMonitor();
	g_pUIMonitor->init("OpenKAI demo", 1980, 1080);
	g_pUIMonitor->addFrame(g_pShow, 0, 0, 1980, 1080);
//	g_pUIMonitor->addFrame(g_pOD->m_pContourFrame, 1280, 0, 700, 360);
//	g_pUIMonitor->addFrame(g_pCamFront->m_pDenseFlow->m_pShowFlow, 1280, 360, 700, 360);
	//TODO: add depth

	//Start threads
	g_pCamFront->start();
	g_pMavlink->start();
//	g_pAP->start();
	g_pOD->start();
//	g_pFD->start();

//UI thread
	g_bRun = true;
	setMouseCallback(g_pCamFront->m_camName, onMouse, NULL);

	while (g_bRun)
	{
		Mavlink_Messages mMsg;
		mMsg = g_pMavlink->current_messages;
		g_pCamFront->m_pCamL->m_bGimbal = true;
		g_pCamFront->m_pCamL->setAttitude(mMsg.attitude.roll, 0,
				mMsg.time_stamps.attitude);

		showScreen();

		g_pUIMonitor->show();

		//Handle key input
		g_key = waitKey(15);
		handleKey(g_key);
	}

	g_pOD->stop();
	g_pFD->stop();
	g_pAP->stop();
	g_pCamFront->stop();
	g_pMavlink->stop();

	g_pOD->complete();
	g_pFD->complete();
	g_pAP->complete();
	g_pCamFront->complete();
	g_pMavlink->complete();
	g_pMavlink->close();

	delete g_pMavlink;
	delete g_pAP;
	delete g_pCamFront;
	delete g_pOD;
	delete g_pFD;

	return 0;

}

void showScreen(void)
{
	int i;
	Mat imMat;
	CamFrame* pFrame = (*g_pCamFront->m_pFrameProcess);

	if (pFrame->getCurrentFrame()->empty())return;
	if (g_pShow->isNewerThan(pFrame))return;

	g_pShow->updateFrame(pFrame);
	g_pShow->getCurrentFrame()->download(imMat);

	OBJECT* pObj;
	for (i = 0; i < g_pOD->m_numObj; i++)
	{
		pObj = &g_pOD->m_pObjects[i];
		if (pObj->m_name[0].empty())
			continue;

		rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(),
				Scalar(0, 255, 0), 2, 5, 0);

		putText(imMat, pObj->m_name[0], pObj->m_boundBox.tl(),
				FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2);
	}

	FAST_OBJECT* pFastObj;
	for (i = 0; i < g_pFD->m_numHuman; i++)
	{
		pFastObj = &g_pFD->m_pHuman[i];

		rectangle(imMat, pFastObj->m_boundBox.tl(), pFastObj->m_boundBox.br(),
				Scalar(0, 0, 255), 2, 5, 0);
	}

	showInfo(&imMat);

	g_pShow->updateFrame(&imMat);
}

#define PUTTEXT(x,y,t) cv::putText(*pDisplayMat, String(t),Point(x, y),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1)

void showInfo(Mat* pDisplayMat)
{
	char strBuf[512];
	std::string strInfo;

	int i;
	int startPosH = 25;
	int startPosV = 25;
	int lineHeight = 20;
	Mavlink_Messages mMsg;

	i = 0;
	mMsg = g_pMavlink->current_messages;

	//Vehicle position
	sprintf(strBuf, "Attitude: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			mMsg.attitude.roll, mMsg.attitude.pitch, mMsg.attitude.yaw);
	PUTTEXT(startPosH, startPosV + lineHeight * (++i), strBuf);

	sprintf(strBuf, "Speed: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			mMsg.attitude.rollspeed, mMsg.attitude.pitchspeed,
			mMsg.attitude.yawspeed);
	PUTTEXT(startPosH, startPosV + lineHeight * (++i), strBuf);

	i++;

	i = 0;
	startPosH = 600;

}

void printEnvironment(void)
{
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
}

void handleKey(int key)
{
	switch (key)
	{
	case 'a':
		break;
	case 'z':
		break;
	case 'c':
		break;
	case 'm':
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
//	if (x > CAM_WIDTH || y > CAM_HEIGHT)return;

	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		break;
	case EVENT_LBUTTONUP:
		break;
	case EVENT_MOUSEMOVE:
		break;
	case EVENT_RBUTTONDOWN:
		break;
	default:
		break;
	}
}

