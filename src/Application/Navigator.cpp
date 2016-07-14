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
	m_waitKey = 50;

	m_bCaffe = 0;
	m_bFCN = 0;

	m_pCamFront = NULL;
	m_pAP = NULL;
	m_pMavlink = NULL;
	m_pCascade = NULL;
	m_pFlow = NULL;
	m_pROITracker = NULL;
	m_pSegNet = NULL;
	m_pDD = NULL;
	m_pMD = NULL;
	m_pClassifier = NULL;
	m_pFCN = NULL;
}

Navigator::~Navigator()
{
}


bool Navigator::start(JSON* pJson)
{
	//TODO: Solve caffe ROI in DepthDetector
	//TODO: Caffe set data to GPU directly
	//TODO: Optimize FCN


	g_pNavigator = this;

	CHECK_INFO(pJson->getVal("APP_SHOW_SCREEN", &m_bShowScreen));
	CHECK_INFO(pJson->getVal("APP_FULL_SCREEN", &m_bFullScreen));
	CHECK_INFO(pJson->getVal("CAFFE_ENABLE", &m_bCaffe));
	CHECK_INFO(pJson->getVal("FCN_ENABLE", &m_bFCN));
	CHECK_INFO(pJson->getVal("APP_WAIT_KEY", &m_waitKey));

	m_pFrame = new CamFrame();

	//Init Camera
	m_pCamFront = new _CamStream();
	CHECK_FATAL(m_pCamFront->init(pJson, "FRONTL"));

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

	if(m_bCaffe)
	{
		//Init Optical Flow
		m_pFlow = new _Flow();
		CHECK_FATAL(m_pFlow->init(pJson, "FRONTL"));
		m_pFlow->m_pCamStream = m_pCamFront;
		m_pCamFront->m_bGray = true;

		//Init Caffe
		m_pClassifier = new _Classifier();
		m_pClassifier->init(pJson);

		//Init Depth Detector
		m_pDD = new _DepthDetector();
		m_pDD->init(pJson,"FRONTL");
		m_pDD->m_pCamStream = m_pCamFront;
		m_pDD->m_pClassifier = m_pClassifier;
		m_pDD->m_pFlow = m_pFlow;

		m_pClassifier->start();
		m_pDD->start();
		m_pFlow->start();

	}

	if(m_bFCN)
	{
		//Init FCN
		m_pFCN = new _FCN();
		m_pFCN->init("",pJson);
		m_pFCN->m_pCamStream = m_pCamFront;

		m_pFCN->start();
	}

	//Start threads
	m_pCamFront->start();
	m_pMavlink->start();
	m_pMD->start();
	m_pAP->start();
	m_pROITracker->start();

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
		m_key = waitKey(m_waitKey);
		handleKey(m_key);
	}


	STOP(m_pAP);
	STOP(m_pMavlink);
	STOP(m_pROITracker);
	STOP(m_pMD);
	STOP(m_pClassifier);
	STOP(m_pDD);
	STOP(m_pFlow);
	STOP(m_pFCN);

	m_pMavlink->complete();
	m_pMavlink->close();

	COMPLETE(m_pAP);
	COMPLETE(m_pROITracker);
	COMPLETE(m_pMD);
	COMPLETE(m_pClassifier);
	COMPLETE(m_pDD);
	COMPLETE(m_pFlow);
	COMPLETE(m_pFCN);
	COMPLETE(m_pCamFront);

	RELEASE(m_pMavlink);
	RELEASE(m_pCamFront);
	RELEASE(m_pROITracker);
	RELEASE(m_pAP);
	RELEASE(m_pMD);
	RELEASE(m_pClassifier);
	RELEASE(m_pDD);
	RELEASE(m_pFlow);
	RELEASE(m_pFCN);

	return 0;

}

void Navigator::showScreen(void)
{
	int i;
	Mat imMat, vMat;
	MARKER_CIRCLE* pCircle;
	fVector2 markerCenter;
	CamFrame* pFrame;


	//Update frames
	pFrame = m_pCamFront->getFrame();
	if(pFrame->empty())return;


	//Acquire the original image
	pFrame->getCMat()->copyTo(m_showMat);
	putText(m_showMat, "Camera FPS: "+f2str(m_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	//Draw Landing Marker
	for(i=0; i<m_pMD->m_numCircle; i++)
	{
		pCircle = &m_pMD->m_pCircle[i];
		circle(m_showMat, Point(pCircle->m_x,pCircle->m_y), pCircle->m_r, Scalar(0, 255, 0), 1);
	}

	markerCenter.m_x = 0;
	markerCenter.m_y = 0;
	if(m_pMD->getCircleCenter(&markerCenter))
	{
		circle(m_showMat, Point(markerCenter.m_x,markerCenter.m_y), 10, Scalar(0, 0, 255), 5);
	}
	putText(m_showMat, "Marker center: ("+f2str(markerCenter.m_x)+" , "+f2str(markerCenter.m_y)+")", cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);


	if(m_bCaffe && !m_pDD->m_Mat.empty())
	{
		vMat = Mat(m_pDD->m_Mat.rows, m_pDD->m_Mat.cols, CV_8UC3, Scalar(0));

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

				drawContours(vMat, contours, -1, Scalar(0, 255, 0),CV_FILLED);

				rectangle(m_showMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), Scalar(0, 255, 0));
				putText(m_showMat, pObj->m_name[0], Point(pObj->m_boundBox.x+pObj->m_boundBox.width/2,
						pObj->m_boundBox.y+pObj->m_boundBox.height/2) ,
						FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
			}

			//Yellow
			if(pObj->m_status == OBJ_CLASSIFYING)
			{
				drawContours(vMat, contours, -1, Scalar(0, 255, 255),1);
	//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), Scalar(0, 255, 255), 1);
			}

			//Red
			if(pObj->m_status == OBJ_ADDED)
			{
				drawContours(vMat, contours, -1, Scalar(0, 0, 255),1);
	//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), Scalar(0, 0, 255), 1);
			}
		}

		cv::resize(vMat, imMat, Size(m_showMat.cols, m_showMat.rows));
		cv::addWeighted(m_showMat, 1.0, imMat, 0.25, 0.0, vMat);
		vMat.copyTo(m_showMat);

		putText(m_showMat, "Caffe FPS: "+f2str(m_pClassifier->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	}


	if(m_bFCN && !m_pFCN->m_segment.empty())
	{
		cv::resize(m_pFCN->m_segment, imMat, Size(m_showMat.cols, m_showMat.rows));
		cv::addWeighted(m_showMat, 1.0, imMat, 0.25, 0.0, vMat);
		vMat.copyTo(m_showMat);

		putText(m_showMat, "FCN FPS: "+f2str(m_pFCN->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	}

	imshow(APP_NAME, m_showMat);

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
