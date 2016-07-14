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
	m_bShowScreen = 1;
	m_bFullScreen = 0;
}

Navigator::~Navigator()
{
}


bool Navigator::start(JSON* pJson)
{
	g_pNavigator = this;

	CHECK_INFO(pJson->getVal("APP_SHOW_SCREEN", &m_bShowScreen));
	CHECK_INFO(pJson->getVal("APP_FULL_SCREEN", &m_bFullScreen));

	//Init Camera
	m_pCamFront = new _CamStream();
	CHECK_FATAL(m_pCamFront->init(pJson, "FRONTL"));

	//Init Optical Flow
	m_pFlow = new _Flow();
	CHECK_FATAL(m_pFlow->init(pJson, "FRONTL"));
	m_pFlow->m_pCamStream = m_pCamFront;
	m_pCamFront->m_bGray = true;

	//Init ROI Tracker
	m_pROITracker = new _ROITracker();
	m_pROITracker->init(pJson, "DRONE");
	m_pROITracker->m_pCamStream = m_pCamFront;

	//Init Marker Detector
	m_pMD = new _MarkerDetector();
	m_pMD->init(pJson,"LANDING");
	m_pMD->m_pCamStream = m_pCamFront;
	m_pCamFront->m_bGray = true;
	m_pCamFront->m_bHSV = true;

	//Init Mavlink
	m_pMavlink = new _MavlinkInterface();
	CHECK_FATAL(m_pMavlink->setup(pJson, "FC"));

	//Init Autopilot
	m_pAP = new _AutoPilot();
	CHECK_FATAL(m_pAP->init(pJson, "_MAIN"));
	m_pAP->m_pMavlink = m_pMavlink;
	m_pAP->m_pROITracker = m_pROITracker;
	m_pAP->m_pMarkerDetector = m_pMD;

	//Init Classifier Manager
	m_pClassifier = new _Classifier();
	m_pClassifier->init(pJson);

	//Init Depth Detector
	m_pDD = new _DepthDetector();
	m_pDD->init(pJson,"FRONTL");
	m_pDD->m_pCamStream = m_pCamFront;
	m_pDD->m_pClassifier = m_pClassifier;
	m_pDD->m_pFlow = m_pFlow;

	//Main window
	m_pFrame = new CamFrame();

	//Start threads
	m_pCamFront->start();
	m_pMavlink->start();
	m_pMD->start();
	m_pAP->start();
	m_pROITracker->start();
	m_pClassifier->start();
	m_pDD->start();
	m_pFlow->start();

	//UI thread
	m_bRun = true;

	if(m_bShowScreen)
	{
		namedWindow(APP_NAME, CV_WINDOW_NORMAL);
		if(m_bFullScreen)
		{
			setWindowProperty(APP_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		}
		setMouseCallback(APP_NAME, onMouseNavigator, NULL);
	}

	while (m_bRun)
	{
//		Mavlink_Messages mMsg;
//		mMsg = m_pMavlink->current_messages;
//		m_pCamFront->m_pCamL->m_bGimbal = true;
//		m_pCamFront->m_pCamL->setAttitude(mMsg.attitude.roll, 0, mMsg.time_stamps.attitude);

		if(m_bShowScreen)
		{
			showScreen();
		}

		//Handle key input
		m_key = waitKey(50);
		handleKey(m_key);
	}

	m_pAP->stop();
	m_pMavlink->stop();
	m_pROITracker->stop();
	m_pMD->stop();
	m_pClassifier->stop();
	m_pDD->stop();
	m_pFlow->stop();

	m_pAP->complete();
	m_pROITracker->complete();
	m_pMavlink->complete();
	m_pMavlink->close();
	m_pMD->complete();
	m_pClassifier->complete();
	m_pDD->complete();
	m_pFlow->complete();
//	m_pCamFront->complete();

	delete m_pMavlink;
	delete m_pCamFront;
	delete m_pROITracker;
	delete m_pAP;
	delete m_pMD;
	delete m_pClassifier;
	delete m_pDD;
	delete m_pFlow;

	return 0;

}

void Navigator::showScreen(void)
{
	int i;
	Mat imMat,imMat2,imMat3,imShow;
	MARKER_CIRCLE* pCircle;
	fVector2 markerCenter;
	CamFrame* pFrame;


	//Update frames
	pFrame = m_pCamFront->getFrame();
	if(pFrame->empty())return;
	if(m_pDD->m_Mat.empty())return;


	//Acquire images
	imMat = *pFrame->getCMat();
	imMat2 = Mat(m_pDD->m_Mat.rows, m_pDD->m_Mat.cols, CV_8UC3, Scalar(0));


	//Draw Landing Marker
	for(i=0; i<m_pMD->m_numCircle; i++)
	{
		pCircle = &m_pMD->m_pCircle[i];
		circle(imMat, Point(pCircle->m_x,pCircle->m_y), pCircle->m_r, Scalar(0, 255, 0), 1);
	}

	markerCenter.m_x = 0;
	markerCenter.m_y = 0;
	if(m_pMD->getCircleCenter(&markerCenter))
	{
		circle(imMat, Point(markerCenter.m_x,markerCenter.m_y), 10, Scalar(0, 0, 255), 5);
	}


	//Draw object contours
	OBJECT* pObj;
	vector< vector< Point > > contours;

	for (i = 0; i < m_pClassifier->m_numObj; i++)
	{
		pObj = &m_pClassifier->m_pObjects[i];
		contours.clear();
		contours.push_back(pObj->m_vContours);

		//Green
		if(pObj->m_status == OBJ_COMPLETE)
		{
			if (pObj->m_name[0].empty())continue;

			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), Scalar(0, 255, 0));
			drawContours(imMat2, contours, -1, Scalar(0, 255, 0),CV_FILLED);

			putText(imMat, pObj->m_name[0], Point(pObj->m_boundBox.x+pObj->m_boundBox.width/2,
					pObj->m_boundBox.y+pObj->m_boundBox.height/2) ,
					FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
		}

		//Yellow
		if(pObj->m_status == OBJ_CLASSIFYING)
		{
			drawContours(imMat2, contours, -1, Scalar(0, 255, 255),1);
//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), Scalar(0, 255, 255), 1);
		}

		//Red
		if(pObj->m_status == OBJ_ADDED)
		{
			drawContours(imMat2, contours, -1, Scalar(0, 0, 255),1);
//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), Scalar(0, 0, 255), 1);
		}
	}

	cv::resize(imMat2, imMat3, Size(imMat.cols,imMat.rows));
	cv::addWeighted(imMat, 1.0, imMat3, 0.25, 0.0, imShow);

	putText(imShow, "Camera FPS: "+f2str(m_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imShow, "Classifier FPS: "+f2str(m_pClassifier->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imShow, "Marker center: ("+f2str(markerCenter.m_x)+" , "+f2str(markerCenter.m_y)+")", cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	imshow(APP_NAME,imShow);

//	if(!m_pDD->m_Mat.empty())
//	{
//		imshow("Contour", m_pDD->m_Mat);
//	}

}

#define PUTTEXT(x,y,t) cv::putText(*pDisplayMat, String(t),Point(x, y),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1)

void Navigator::showInfo(Mat* pDisplayMat)
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
//		m_pUIMonitor->removeAll();
//		m_pUIMonitor->addFrame(m_pShow, 0, 0, 1980, 1080);
		break;
	case 'w':
//		m_pUIMonitor->removeAll();
//		m_pUIMonitor->addFrame(m_pOD->m_pContourFrame, 0,0,1980,1080);
		break;
//	case 'e':
//		m_pUIMonitor->removeAll();
//		m_pUIMonitor->addFrame(m_pCamFront->m_pDenseFlow->m_pShowFlow, 0, 0, 1980, 1080);
//		break;
	case 'r':
//		m_pUIMonitor->removeAll();
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
