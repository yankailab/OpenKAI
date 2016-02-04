#include "demo.h"

int main(int argc, char* argv[])
{
	//Init Logger
	google::InitGoogleLogging(argv[0]);
	printEnvironment();

	setDevice(0);

	//Load config
	LOG(INFO)<<"Using config file: "<<argv[1];
	printf(argv[1]);
	printf("\n");
	CHECK_FATAL(g_file.open(argv[1]));
	string config = g_file.getContent();
	CHECK_FATAL(g_Json.parse(config.c_str()));

	//Connect to Mavlink
	g_pMavlink = new _MavlinkInterface();
	CHECK_FATAL(g_pMavlink->setup(&g_Json, "FC"));
	CHECK_INFO(g_pMavlink->open());

	//Init Classifier Manager
//	g_pClassMgr = new _ClassifierManager();
//	g_pClassMgr->init(&g_Json);

	//Init Object Detector
	g_pOD = new _ObjectDetector();
	g_pOD->init(&g_Json);
	g_pOD->m_pClassMgr = g_pClassMgr;
	//Temporal
	g_pOD->m_bOneImg = 1;

	//Init Camera
	g_pCamFront = new _CamStream();
	CHECK_FATAL(g_pCamFront->init(&g_Json, "FRONTL"));
	g_pCamFront->m_bGray = true;

	//Init Optical Flow
	g_pDF = new _DenseFlow();
	CHECK_FATAL(g_pDF->init(&g_Json, "FRONTL"));
	g_pDF->m_pGray = g_pCamFront->m_pGrayL;
	g_pDF->m_pCamStream = g_pCamFront;

	//Init Fast Detector
	g_pFD = new _CascadeDetector();
	g_pFD->init("DRONE", &g_Json);
	g_pFD->m_pGray = g_pCamFront->m_pGrayL;
	g_pFD->m_pCamStream = g_pCamFront;

	//Init SegNet
	g_pSegNet = new _SegNet();
	g_pSegNet->init("DEFAULT",&g_Json);
	g_pSegNet->m_pFrame = g_pCamFront->m_pFrameL;
	g_pSegNet->m_pCamStream = g_pCamFront;

	//Init Autopilot
	g_pAP = new _AutoPilot();
	CHECK_FATAL(g_pAP->setup(&g_Json, ""));
	g_pAP->init();
	g_pAP->setCamStream(g_pCamFront, CAM_FRONT);
	g_pAP->m_pOD = g_pOD;
	g_pAP->m_pFD = g_pFD;
//	g_pMD = g_pAP->m_pCamStream[CAM_FRONT].m_pCam->m_pMarkerDetect;

	//Main window
	g_pShow = new CamFrame();
	g_pMat = new CamFrame();
	g_pMat2 = new CamFrame();

	//Init UI Monitor
	g_pUIMonitor = new UIMonitor();
	g_pUIMonitor->init("OpenKAI demo", &g_Json);
	g_pUIMonitor->addFullFrame(g_pShow);

	//Start threads
	g_pCamFront->start();
//	g_pMavlink->start();
//	g_pClassMgr->start();
//	g_pOD->start();
//	g_pDF->start();
//	g_pAP->start();
	g_pFD->start();
//	g_pSegNet->start();

	//UI thread
	g_bRun = true;
	setMouseCallback(g_pCamFront->m_camName, onMouse, NULL);

	while (g_bRun)
	{
//		Mavlink_Messages mMsg;
//		mMsg = g_pMavlink->current_messages;
//		g_pCamFront->m_pCamL->m_bGimbal = true;
//		g_pCamFront->m_pCamL->setAttitude(mMsg.attitude.roll, 0, mMsg.time_stamps.attitude);

		showScreen();

		//Handle key input
		g_key = waitKey(30);
		handleKey(g_key);
	}

	g_pDF->stop();
	g_pOD->stop();
	g_pFD->stop();
	g_pAP->stop();
	g_pCamFront->stop();
	g_pMavlink->stop();
	g_pClassMgr->stop();
	g_pSegNet->stop();

	g_pDF->complete();
	g_pClassMgr->complete();
	g_pOD->complete();
	g_pFD->complete();
	g_pAP->complete();
	g_pCamFront->complete();
	g_pMavlink->complete();
	g_pMavlink->close();

//	delete g_pClassMgr;
//	delete g_pAP;
//	delete g_pMavlink;
	delete g_pDF;
	delete g_pCamFront;
	delete g_pOD;
	delete g_pFD;

	return 0;

}

void showScreen(void)
{
	int i;
	Mat imMat,imMat2,imMat3;
	CamFrame* pFrame = (*g_pCamFront->m_pFrameProcess);

	if (pFrame->getCurrentFrame()->empty())return;
	if (g_pShow->isNewerThan(pFrame))return;
//	if (g_pSegNet->m_segment.empty())return;

	pFrame->getCurrentFrame()->download(imMat);

//	g_pMat->updateFrame(&g_pSegNet->m_segment);
//	g_pMat2->getResizedOf(g_pMat, imMat.cols,imMat.rows);
//	g_pMat2->getCurrentFrame()->download(imMat2);

	CASCADE_OBJECT* pDrone;
	int iTarget = 0;

	for (i = 0; i < g_pFD->m_numObj; i++)
	{
		pDrone = &g_pFD->m_pObj[i];
		if(pDrone->m_status != OBJ_ADDED)continue;

		if(iTarget == 0)iTarget = i;
		rectangle(imMat, pDrone->m_boundBox.tl(), pDrone->m_boundBox.br(), Scalar(0, 0, 255), 2);
	}

	pDrone = &g_pFD->m_pObj[iTarget];
	putText(imMat, "LOCK: DJI Phantom", Point(pDrone->m_boundBox.tl().x,pDrone->m_boundBox.tl().y-20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);

//	cv::addWeighted(imMat, 1.0, imMat2, 0.5, 0.0, imMat3);

	putText(imMat, "Camera FPS: "+f2str(g_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imMat, "Cascade FPS: "+f2str(g_pFD->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat3, "SegNet FPS: "+f2str(g_pSegNet->getFrameRate()), cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

//	g_pShow->updateFrame(&imMat3);
	imshow("OpenKAI demo",imMat);

	if(!g_pDF->m_showMat.empty())
	{
//		imshow("DenseFlow",g_pDF->m_showMat);
	}

//	g_pUIMonitor->show();

	return;


/*
	int i;
	Mat imMat;
	vector< vector< Point > > contours;
	CamFrame* pFrame = (*g_pCamFront->m_pFrameProcess);

	if (pFrame->getCurrentFrame()->empty())return;
	if (g_pShow->isNewerThan(pFrame))return;

	g_pShow->updateFrame(pFrame);
	g_pShow->getCurrentFrame()->download(imMat);

	OBJECT* pObj;
	for (i = 0; i < g_pClassMgr->m_numObj; i++)
	{
		pObj = &g_pClassMgr->m_pObjects[i];
		contours.clear();
		contours.push_back(pObj->m_vContours);

		//Green
		if(pObj->m_status == OBJ_COMPLETE)
		{
			if (pObj->m_name[0].empty())continue;

			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(),
					Scalar(0, 255, 0), 2, 5, 0);

//			drawContours(imMat, contours, -1, Scalar(0, 255, 0),2);

			putText(imMat, pObj->m_name[0], pObj->m_boundBox.tl(),
					FONT_HERSHEY_SIMPLEX, 0.9, Scalar(255, 0, 0), 2);
		}

		//Yellow
		if(pObj->m_status == OBJ_CLASSIFYING)
		{
//			drawContours(imMat, contours, -1, Scalar(0, 255, 255),1);
			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(),
					Scalar(0, 255, 255), 1);
		}

		//Red
//		if(pObj->m_status == OBJ_ADDED)
//		{
////			drawContours(imMat, contours, -1, Scalar(0, 0, 255),1);
//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(),
//					Scalar(0, 0, 255), 1);
//		}
	}


	FAST_OBJECT* pFastObj;
	for (i = 0; i < g_pFD->m_numHuman; i++)
	{
		pFastObj = &g_pFD->m_pHuman[i];
		if(pFastObj->m_status != OBJ_ADDED)continue;

		rectangle(imMat, pFastObj->m_boundBox.tl(), pFastObj->m_boundBox.br(),
				Scalar(0, 0, 255), 1);
	}

	showInfo(&imMat);

	g_pShow->updateFrame(&imMat);
	*/
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
	case 'q':
		g_pUIMonitor->removeAll();
		g_pUIMonitor->addFrame(g_pShow, 0, 0, 1980, 1080);
		break;
	case 'w':
		g_pUIMonitor->removeAll();
		g_pUIMonitor->addFrame(g_pOD->m_pContourFrame, 0,0,1980,1080);
		break;
//	case 'e':
//		g_pUIMonitor->removeAll();
//		g_pUIMonitor->addFrame(g_pCamFront->m_pDenseFlow->m_pShowFlow, 0, 0, 1980, 1080);
//		break;
	case 'r':
		g_pUIMonitor->removeAll();
		g_pUIMonitor->addFrame(g_pOD->m_pSaliencyFrame, 0, 0, 1980, 1080);
		break;
	case 't':
		g_pOD->m_bOneImg = 1 - g_pOD->m_bOneImg;
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

