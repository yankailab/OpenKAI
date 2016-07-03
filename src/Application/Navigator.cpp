#include "Navigator.h"

Navigator* g_pNavigator;
void onMouseNavigator(int event, int x, int y, int flags, void* userdata)
{
	g_pNavigator->handleMouse(event,x,y,flags);
}


namespace kai
{

Navigator::Navigator()
{
}

Navigator::~Navigator()
{
}


bool Navigator::start(JSON* pJson)
{
	g_pNavigator = this;

	//Init Camera
	m_pCamFront = new _CamStream();
	CHECK_FATAL(m_pCamFront->init(pJson, "FRONTL"));

	//Init Optical Flow
//	m_pDF = new _DenseFlow();
//	CHECK_FATAL(m_pDF->init(pJson, "FRONTL"));
//	m_pDF->m_pCamStream = m_pCamFront;
//	m_pCamFront->m_bGray = true;

	//Init Dense Flow Tracker
//	m_p3DFlow = new _3DFlow();
//	m_p3DFlow->init(pJson, "DEFAULT");
//	m_p3DFlow->m_pDF = m_pDF;

	//Init ROI Tracker
	m_pROITracker = new _ROITracker();
	m_pROITracker->init(pJson, "DRONE");
	m_pROITracker->m_pCamStream = m_pCamFront;

	//Init Depth Detector
//	m_pDD = new _DepthDetector();
//	m_pDD->init(pJson,"FRONTL");
//	m_pDD->m_pCamStream = m_pCamFront;

	//Init Marker Detector
	m_pMD = new _MarkerDetector();
	m_pMD->init(pJson,"FRONTL");
	m_pMD->m_pCamStream = m_pCamFront;

	//Init Mavlink
	m_pMavlink = new _MavlinkInterface();
	CHECK_FATAL(m_pMavlink->setup(pJson, "FC"));

	//Init Autopilot
	m_pAP = new _AutoPilot();
	CHECK_FATAL(m_pAP->init(pJson, "_MAIN"));
	m_pAP->m_pMavlink = m_pMavlink;
	m_pAP->m_pROITracker = m_pROITracker;
	m_pAP->m_pMarkerDetector = m_pMD;

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
	m_pMavlink->start();
	m_pMD->start();
	m_pAP->start();
	m_pROITracker->start();
//	m_pDF->start();
//	m_p3DFlow->start();
//	m_pDD->start();

	//UI thread
	m_bRun = true;
	namedWindow(APP_NAME, CV_WINDOW_NORMAL);
	setWindowProperty(APP_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	setMouseCallback(APP_NAME, onMouseNavigator, NULL);

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

	m_pAP->stop();
	m_pMavlink->stop();
	m_pROITracker->stop();
	m_pMD->stop();
//	m_pDF->stop();
//	m_p3DFlow->stop();
//	m_pDD->stop();

	m_pAP->complete();
	m_pROITracker->complete();
	m_pMavlink->complete();
	m_pMavlink->close();
	m_pMD->complete();
//	m_pDF->complete();
//	m_p3DFlow->complete();
//	m_pDD->complete();
//	m_pCamFront->complete();

	delete m_pMavlink;
	delete m_pCamFront;
	delete m_pROITracker;
	delete m_pAP;
	delete m_pMD;
//	delete m_pDD;
//	delete m_pDF;
//	delete m_p3DFlow;

	return 0;

}

void Navigator::showScreen(void)
{
	int i;
	Mat imMat,imMat2,imMat3;
	CamFrame* pFrame = m_pCamFront->getFrame();

	if (pFrame->empty())return;
	imMat = *pFrame->getCMat();

//	if(m_p3DFlow->m_pDepth->empty())return;
//
//	m_pMat->getResizedOf(m_p3DFlow->m_pSeg, imMat.cols,imMat.rows);
//	m_pMat2->get8UC3Of(m_pMat);
//	imMat2 = *m_pMat2->getCMat();
//
//	cv::addWeighted(imMat, 1.0, imMat2, 0.35, 0.0, imMat3);
//
//	 // draw the tracked object
//	Rect roi = m_pROITracker->m_ROI;
//	if(roi.height>0 || roi.width>0)
//	{
//		rectangle( imMat3, roi, Scalar( 0, 0, 255 ), 2 );
//	}

//	if(m_pDD->m_pDepth->empty())return;

	putText(imMat, "Camera FPS: "+f2str(m_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat3, "DenseFlow FPS: "+f2str(m_pDF->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat3, "FlowDepth FPS: "+f2str(m_p3DFlow->getFrameRate()), cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat3, "ROITracker FPS: "+f2str(m_pROITracker->getFrameRate()), cv::Point(15,75), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat3, "Cascade FPS: "+f2str(m_pCascade->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	imshow(APP_NAME,imMat);
//	imshow("Depth",*m_pDFDepth->m_pDepth->getCMat());
//	imshow(APP_NAME,*m_pDD->m_pDepth->getCMat());


//	g_pShow->updateFrame(&imMat3);
//	g_pUIMonitor->show();

}

#define PUTTEXT(x,y,t) cv::putText(*pDisplayMat, String(t),Point(x, y),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1)

void Navigator::showInfo(UMat* pDisplayMat)
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

void Navigator::handleKey(int key)
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

void Navigator::handleMouse(int event, int x, int y, int flags)
{
	Rect2d roi;
	int roisize = 200;

	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		roi.x = x - roisize/2;
		roi.y = y - roisize/2;
		roi.width = roisize;
		roi.height = roisize;
		m_pROITracker->setROI(roi);
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
