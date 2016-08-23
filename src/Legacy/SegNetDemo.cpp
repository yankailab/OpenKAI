#include "SegNetDemo.h"

SegNetDemo* g_pSegNetDemo;
void onMouseSegNet(int event, int x, int y, int flags, void* userdata)
{
	g_pSegNetDemo->handleMouse(event,x,y,flags);
}


namespace kai
{

SegNetDemo::SegNetDemo()
{
}

SegNetDemo::~SegNetDemo()
{
}


bool SegNetDemo::start(JSON* pJson)
{
	g_pSegNetDemo = this;
	m_showMode = 0;

	//Init Camera
	m_pCamFront = new _Stream();
	CHECK_FATAL(m_pCamFront->init(pJson, "FRONTL"));

	//Init SegNet
	m_pSegNet = new _SegNet();
	m_pSegNet->init("DEFAULT",pJson);
	m_pSegNet->m_pCamStream = m_pCamFront;
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
	m_pShow = new Frame();
	m_pMat = new Frame();
	m_pMat2 = new Frame();

	//Init UI Monitor
	m_pUIMonitor = new UIMonitor();
	m_pUIMonitor->init("OpenKAI demo", pJson);
	m_pUIMonitor->addFullFrame(m_pShow);

	//Start threads
	m_pCamFront->start();
//	m_pMavlink->start();
//	m_pDF->start();
//	m_pAP->start();
	m_pSegNet->start();

	//UI thread
	m_bRun = true;
	namedWindow(APP_NAME, CV_WINDOW_NORMAL);
	setWindowProperty(APP_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	setMouseCallback(APP_NAME, onMouseSegNet, NULL);

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
	m_pSegNet->stop();
//	m_pMavlink->stop();

	m_pSegNet->complete();
//	m_pDF->complete();
//	m_pAP->complete();
//	m_pCamFront->complete();
//	m_pMavlink->complete();
//	m_pMavlink->close();

//	delete m_pAP;
//	delete m_pMavlink;
//	delete m_pDF;
	delete m_pCamFront;
	delete m_pSegNet;

	return 0;

}

void SegNetDemo::showScreen(void)
{
	int i;
	Mat imMat,imMat2,imMat3;
	Frame* pFrame = m_pCamFront->getBGRFrame();// (*m_pCamFront->m_pFrameProcess);

	if (pFrame->getGMat()->empty())return;
	if (m_pSegNet->m_segment.empty())return;

	pFrame->getGMat()->download(imMat);

	m_pMat->update(&m_pSegNet->m_segment);
	m_pMat2->getResizedOf(m_pMat, imMat.cols,imMat.rows);
	m_pMat2->getGMat()->download(imMat2);

	cv::addWeighted(imMat, 1.0, imMat2, 0.5, 0.0, imMat3);

	putText(imMat3, "Camera FPS: "+f2str(m_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imMat3, "Segmentation FPS: "+f2str(m_pSegNet->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	if(m_showMode == 0)
	{
		imshow(APP_NAME,imMat);
	}
	else
	{
		imshow(APP_NAME,imMat3);
	}

}

#define PUTTEXT(x,y,t) cv::putText(*pDisplayMat, String(t),Point(x, y),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1)

void SegNetDemo::showInfo(UMat* pDisplayMat)
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

void SegNetDemo::handleKey(int key)
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

void SegNetDemo::handleMouse(int event, int x, int y, int flags)
{

	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		break;
	case EVENT_LBUTTONUP:
		m_showMode = 1 - m_showMode;
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
