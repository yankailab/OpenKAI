#include "DroneHunter.h"

DroneHunter* g_pDroneHunter;
void onMouseDroneHunter(int event, int x, int y, int flags, void* userdata)
{
	g_pDroneHunter->handleMouse(event,x,y,flags);
}


namespace kai
{

DroneHunter::DroneHunter()
{
}

DroneHunter::~DroneHunter()
{
}


bool DroneHunter::start(JSON* pJson)
{
	g_pDroneHunter = this;

	//Init Camera
	m_pCamFront = new _CamStream();
	CHECK_FATAL(m_pCamFront->init(pJson, "FRONTL"));

	//Init Fast Detector
	m_pCascade = new _CascadeDetector();
	m_pCascade->init("DRONE", pJson);
	m_pCascade->m_pCamStream = m_pCamFront;
	m_pCamFront->m_bGray = true;

	//Init Optical Flow
	m_pDF = new _DenseFlow();
	CHECK_FATAL(m_pDF->init(pJson, "FRONTL"));
	m_pDF->m_pCamStream = m_pCamFront;
	m_pCamFront->m_bGray = true;

	//Init Autopilot
/*	m_pAP = new _AutoPilot();
	CHECK_FATAL(m_pAP->setup(&m_Json, ""));
	m_pAP->init();
	m_pAP->setCamStream(m_pCamFront, CAM_FRONT);
	m_pAP->m_pOD = m_pOD;
	m_pAP->m_pFD = m_pFD;
//	m_pMD = m_pAP->m_pCamStream[CAM_FRONT].m_pCam->m_pMarkerDetect;
*/

	//Connect to Mavlink
/*	m_pMavlink = new _MavlinkInterface();
	CHECK_FATAL(m_pMavlink->setup(&m_Json, "FC"));
	CHECK_INFO(m_pMavlink->open());
*/

	//Main window
	m_pShow = new CamFrame();
	m_pMat = new CamFrame();
	m_pMat2 = new CamFrame();

	//Init UI Monitor
//	m_pUIMonitor = new UIMonitor();
//	m_pUIMonitor->init("OpenKAI demo", pJson);
//	m_pUIMonitor->addFullFrame(m_pShow);

	//Start threads
	m_pCamFront->start();
//	m_pMavlink->start();
	m_pDF->start();
//	m_pAP->start();
	m_pCascade->start();

	//UI thread
	m_bRun = true;
//	namedWindow(APP_NAME, CV_WINDOW_NORMAL);
//	setWindowProperty(APP_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
//	setMouseCallback(APP_NAME, onMouseDroneHunter, NULL);

	while (m_bRun)
	{
//		Mavlink_Messages mMsg;
//		mMsg = m_pMavlink->current_messages;
//		m_pCamFront->m_pCamL->m_bGimbal = true;
//		m_pCamFront->m_pCamL->setAttitude(mMsg.attitude.roll, 0, mMsg.time_stamps.attitude);

		showScreen();

		//Handle key input
		m_key = waitKey(30);
		handleKey(m_key);
	}

//	m_pAP->stop();
	m_pCascade->stop();
//	m_pMavlink->stop();
	m_pDF->stop();

	m_pCascade->complete();
	m_pDF->complete();
//	m_pAP->complete();
//	m_pCamFront->complete();
//	m_pMavlink->complete();
//	m_pMavlink->close();

//	delete m_pAP;
//	delete m_pMavlink;
	delete m_pDF;
	delete m_pCamFront;
	delete m_pCascade;

	return 0;

}

void DroneHunter::showScreen(void)
{
	int i;
	Mat imMat;
	CamFrame* pFrame = m_pCamFront->getFrame();// (*m_pCamFront->m_pFrameProcess);

	if (pFrame->empty())return;
	pFrame->getGMat()->download(imMat);

	CASCADE_OBJECT* pDrone;
	int iTarget = 0;

	for (i = 0; i < m_pCascade->m_numObj; i++)
	{
		pDrone = &m_pCascade->m_pObj[i];
		if(pDrone->m_status != OBJ_ADDED)continue;

		if(iTarget == 0)iTarget = i;
		rectangle(imMat, pDrone->m_boundBox.tl(), pDrone->m_boundBox.br(), Scalar(0, 0, 255), 2);
	}

	pDrone = &m_pCascade->m_pObj[iTarget];
	putText(imMat, "LOCK: DJI Phantom", Point(pDrone->m_boundBox.tl().x,pDrone->m_boundBox.tl().y-20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);

	putText(imMat, "Camera FPS: "+f2str(m_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imMat, "Cascade FPS: "+f2str(m_pCascade->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	imshow(APP_NAME,imMat);

	if(!m_pDF->m_showMat.empty())
	{
		imshow("OpticalFlow",m_pDF->m_showMat);
	}

//	g_pShow->updateFrame(&imMat3);
//	g_pUIMonitor->show();

}

#define PUTTEXT(x,y,t) cv::putText(*pDisplayMat, String(t),Point(x, y),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1)

void DroneHunter::showInfo(UMat* pDisplayMat)
{
	char strBuf[512];
	std::string strInfo;

	int i;
	int startPosH = 25;
	int startPosV = 25;
	int lineHeight = 20;
	Mavlink_Messages mMsg;

	i = 0;
	mMsg = m_pMavlink->current_messages;

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

void DroneHunter::handleKey(int key)
{
	switch (key)
	{
	case 'q':
		m_pUIMonitor->removeAll();
		m_pUIMonitor->addFrame(m_pShow, 0, 0, 1980, 1080);
		break;
	case 'w':
		m_pUIMonitor->removeAll();
//		m_pUIMonitor->addFrame(m_pOD->m_pContourFrame, 0,0,1980,1080);
		break;
//	case 'e':
//		m_pUIMonitor->removeAll();
//		m_pUIMonitor->addFrame(m_pCamFront->m_pDenseFlow->m_pShowFlow, 0, 0, 1980, 1080);
//		break;
	case 'r':
		m_pUIMonitor->removeAll();
//		m_pUIMonitor->addFrame(m_pOD->m_pSaliencyFrame, 0, 0, 1980, 1080);
		break;
	case 't':
//		m_pOD->m_bOneImg = 1 - m_pOD->m_bOneImg;
		break;
	case 27:
		m_bRun = false;	//ESC
		break;
	default:
		break;
	}
}

void DroneHunter::handleMouse(int event, int x, int y, int flags)
{

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

}
