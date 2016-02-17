#include "ExtCamControl.h"

ExtCamControl* g_pExtCamControl;

void onMouseExtCamControl(int event, int x, int y, int flags, void* userdata)
{
	g_pExtCamControl->handleMouse(event,x,y,flags);
}

namespace kai
{

ExtCamControl::ExtCamControl()
{
}

ExtCamControl::~ExtCamControl()
{
}

bool ExtCamControl::start(JSON* pJson)
{
	g_pExtCamControl = this;

	//Init Camera
	m_pCamFront = new _CamStream();
	CHECK_FATAL(m_pCamFront->init(pJson, "FRONTL"));

	//Init Marker Detector
	m_pMD = new _MarkerDetector();
	CHECK_FATAL(m_pMD->init(pJson, "RED_CIRCLE"));
	m_pMD->m_pCamStream = m_pCamFront;
	m_pCamFront->m_bHSV = true;

	//Init BgFg Detector
	m_pBgFgD = new _BgFgDetector();
	CHECK_FATAL(m_pBgFgD->init(pJson, ""));
	m_pBgFgD->m_pCamStream = m_pCamFront;

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
	m_pUIMonitor = new UIMonitor();
	m_pUIMonitor->init("OpenKAI demo", pJson);
	m_pUIMonitor->addFullFrame(m_pShow);

	//Start threads
	m_pCamFront->start();
//	m_pMavlink->start();
//	m_pDF->start();
	m_pMD->start();
//	m_pAP->start();
	m_pBgFgD->start();

	//UI thread
	m_bRun = true;
	namedWindow(APP_NAME, CV_WINDOW_NORMAL);
	setWindowProperty(APP_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	setMouseCallback(APP_NAME, onMouseExtCamControl, NULL);

	while (m_bRun)
	{
//		Mavlink_Messages mMsg;
//		mMsg = m_pMavlink->current_messages;
//		m_pCamFront->m_pCamL->m_bGimbal = true;
//		m_pCamFront->m_pCamL->setAttitude(mMsg.attitude.roll, 0, mMsg.time_stamps.attitude);

		if(!showScreenMarkerDetector())
		{
			showScreenBgFgDetector();
		}

		//Handle key input
		m_key = waitKey(30);
		handleKey(m_key);
	}

//	m_pAP->stop();
	m_pMD->stop();
//	m_pMavlink->stop();
	m_pBgFgD->stop();

	m_pMD->complete();
	m_pBgFgD->complete();
//	m_pDF->complete();
//	m_pAP->complete();
//	m_pCamFront->complete();
//	m_pMavlink->complete();
//	m_pMavlink->close();

//	delete m_pAP;
//	delete m_pMavlink;
//	delete m_pDF;
	delete m_pCamFront;

	return 0;

}

bool ExtCamControl::showScreenMarkerDetector(void)
{
	int i;
	UMat imMat;
	CamFrame* pFrame = m_pCamFront->getLastFrame();

	if (pFrame->getGMat()->empty())return false;
	pFrame->getGMat()->download(imMat);

	putText(imMat, "Camera FPS: "+f2str(m_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imMat, "Marker Detector FPS: "+f2str(m_pMD->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	fVector3 markerPos;
	int lockLevel = m_pMD->getObjLockLevel();
	if (lockLevel < LOCK_LEVEL_POS)
	{
		return false;
	}

	putText(imMat, "MARKER LOCKED", cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	//Update current position with trajectory estimation
	m_pMD->getObjPosition(&markerPos);
	circle(imMat, Point(markerPos.m_x,markerPos.m_y), markerPos.m_z, Scalar(0, 255, 0), 2);

	imshow(APP_NAME,imMat);
	return true;
}

void ExtCamControl::showScreenBgFgDetector(void)
{
	int i;
	UMat imMat, imMat2;
	CamFrame* pFrame = m_pCamFront->getLastFrame();

	if(pFrame->getGMat()->empty())return;
	if(m_pBgFgD->m_Mat.empty())return;
	if(m_pBgFgD->m_gFg.empty())return;

//	m_pBgFgD->m_gFg.download(imMat2);
//	imshow("Bg",imMat);
//	imshow("Fg",imMat2);
//	imshow("BgFgContour",m_pBgFgD->m_Mat);

	imMat = m_pBgFgD->m_Mat;

	putText(imMat, "Camera FPS: "+f2str(m_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imMat, "BgFg Detector FPS: "+f2str(m_pBgFgD->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

//	fVector3 markerPos;
//	int lockLevel = m_pMD->getObjLockLevel();
//	if (lockLevel >= LOCK_LEVEL_POS)
//	{
//		putText(imMat, "BgFg", cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//		//Update current position with trajectory estimation
//		m_pMD->getObjPosition(&markerPos);
//		circle(imMat, Point(markerPos.m_x,markerPos.m_y), markerPos.m_z, Scalar(0, 255, 0), 2);
//	}

	imshow(APP_NAME,imMat);
}

#define PUTTEXT(x,y,t) cv::putText(*pDisplayMat, String(t),Point(x, y),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1)

void ExtCamControl::showInfo(UMat* pDisplayMat)
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

void ExtCamControl::handleKey(int key)
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

void ExtCamControl::handleMouse(int event, int x, int y, int flags)
{
	fVector3 pos;
	pos.m_x = x;
	pos.m_y = y;
	pos.m_z = 0;

	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		m_pMD->setObjROI(pos);
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
