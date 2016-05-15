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

	CHECK_ERROR(pJson->getVal("ROI_MIN_SIZE", &m_minROI));

	//Init Camera
	m_pCamFront = new _CamStream();
	CHECK_FATAL(m_pCamFront->init(pJson, "FRONTL"));

//	//Init Fast Detector
//	m_pCascade = new _CascadeDetector();
//	m_pCascade->init("DRONE", pJson);
//	m_pCascade->m_pCamStream = m_pCamFront;
//	m_pCamFront->m_bGray = true;
//
//	//Init Feature Detector
//	m_pFeature = new _FeatureDetector();
//	m_pFeature->init("DRONE", pJson);
//	m_pFeature->m_pCamStream = m_pCamFront;
//	m_pCamFront->m_bGray = true;

	//Init ROI Tracker
	m_pROITracker = new _ROITracker();
	m_pROITracker->init(pJson, "DRONE");
	m_pROITracker->m_pCamStream = m_pCamFront;

	//Init Optical Flow
//	m_pDF = new _DenseFlow();
//	CHECK_FATAL(m_pDF->init(pJson, "FRONTL"));
//	m_pDF->m_pCamStream = m_pCamFront;
//	m_pCamFront->m_bGray = true;

	//Init Dense Flow Tracker
//	m_pDFDepth = new _DenseFlowDepth();
//	m_pDFDepth->init(pJson, "DEFAULT");
//	m_pDFDepth->m_pDF = m_pDF;

	//Connect to Mavlink
	m_pVlink = new _VehicleInterface();
	CHECK_FATAL(m_pVlink->setup(pJson, "FC"));
	CHECK_INFO(m_pVlink->open());

	//Init Autopilot
	m_pAP = new _AutoPilot();
	CHECK_FATAL(m_pAP->init(pJson, "_MAIN"));
	m_pAP->m_pVI = m_pVlink;
	m_pAP->m_pROITracker = m_pROITracker;

	m_ROI.m_x = 0;
	m_ROI.m_y = 0;
	m_ROI.m_z = 0;
	m_ROI.m_w = 0;
	m_bSelect = false;
	m_ROImode = MODE_ASSIST;
	m_ROIsize = 100;
	m_ROIsizeFrom = 50;
	m_ROIsizeTo = 300;

	m_btnSize = 100;
	m_btnROIClear = 100;
	m_btnROIBig = 200;
	m_btnROISmall = 300;
	m_btnMode = 980;

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
//	m_pFeature->start();
//	m_pVlink->start();
//	m_pDF->start();
	m_pAP->start();
//	m_pCascade->start();
//	m_pDFDepth->start();
	m_pROITracker->start();

	//UI thread
	m_bRun = true;
	namedWindow(APP_NAME, CV_WINDOW_NORMAL);
	setWindowProperty(APP_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	setMouseCallback(APP_NAME, onMouseDroneHunter, NULL);

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
//	m_pCascade->stop();
	m_pVlink->stop();
//	m_pDF->stop();
//	m_pFeature->stop();
//	m_pDFDepth->stop();
	m_pROITracker->stop();

//	m_pCascade->complete();
//	m_pDF->complete();
//	m_pFeature->complete();
//	m_pDFDepth->complete();
	m_pROITracker->complete();
	m_pAP->complete();
//	m_pCamFront->complete();
	m_pVlink->complete();
	m_pVlink->close();

	delete m_pAP;
	delete m_pVlink;
//	delete m_pDF;
//	delete m_pCamFront;
//	delete m_pCascade;
//	delete m_pFeature;
//	delete m_pDFDepth;
	delete m_pROITracker;

	return 0;

}

void DroneHunter::showScreen(void)
{
	int i;
	Rect roi;
	Mat imMat,imMat2,imMat3;
	CamFrame* pFrame = m_pCamFront->getFrame();

	if (pFrame->empty())return;
	pFrame->getGMat()->download(imMat);

	 // draw the tracked object
	if(m_bSelect)
	{
		roi = getROI(m_ROI);
		if(roi.height>0 || roi.width>0)
		{
			rectangle( imMat, roi, Scalar( 0, 255, 0 ), 2 );
		}
	}
	else if(m_pROITracker->m_bTracking)
	{
		roi = m_pROITracker->m_ROI;
		if(roi.height>0 || roi.width>0)
		{
			rectangle( imMat, roi, Scalar( 0, 0, 255 ), 2 );
		}
	}

	putText(imMat, "Camera FPS: "+f2str(m_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imMat, "ROITracker FPS: "+f2str(m_pROITracker->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imMat, "AutoPilot FPS: "+f2str(m_pAP->getFrameRate()), cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	putText(imMat, "Roll: "+f2str(m_pAP->m_RC[0]), cv::Point(15,75), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	putText(imMat, "Pitch: "+f2str(m_pAP->m_RC[1]), cv::Point(15,95), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	if(m_ROImode == MODE_ASSIST)
	{
		putText(imMat, "CLR", cv::Point(1825,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1);
		putText(imMat, "+", cv::Point(1825,150), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1);
		putText(imMat, "-", cv::Point(1825,250), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1);

		roi.x = imMat.cols - m_btnSize;
		roi.y = 0;
		roi.width = m_btnSize;
		roi.height = m_btnSize;
		rectangle( imMat, roi, Scalar( 0, 255, 0 ), 1 );

		roi.y = m_btnROIClear;
		rectangle( imMat, roi, Scalar( 0, 255, 0 ), 1 );

		roi.y = m_btnROIBig;
		rectangle( imMat, roi, Scalar( 0, 255, 0 ), 1 );
	}

	putText(imMat, "MODE", cv::Point(1825,1035), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1);

	roi.x = imMat.cols - m_btnSize;
	roi.y = m_btnMode;
	roi.width = m_btnSize;
	roi.height = m_btnSize;
	rectangle( imMat, roi, Scalar( 0, 255, 0 ), 1 );


	circle(imMat, Point(m_pAP->m_roll.m_targetPos, m_pAP->m_pitch.m_targetPos), 50, Scalar(0,255,0), 2);
	imshow(APP_NAME,imMat);

	//	CASCADE_OBJECT* pDrone;
	//	int iTarget = 0;
	//
	//	for (i = 0; i < m_pCascade->m_numObj; i++)
	//	{
	//		pDrone = &m_pCascade->m_pObj[i];
	//		if(pDrone->m_status != OBJ_ADDED)continue;
	//
	//		if(iTarget == 0)iTarget = i;
	//		rectangle(imMat, pDrone->m_boundBox.tl(), pDrone->m_boundBox.br(), Scalar(0, 0, 255), 2);
	//	}
	//
	//	pDrone = &m_pCascade->m_pObj[iTarget];
	//	putText(imMat, "LOCK: DJI Phantom", Point(pDrone->m_boundBox.tl().x,pDrone->m_boundBox.tl().y-20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);

	//	putText(imMat, "Cascade FPS: "+f2str(m_pCascade->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	//	putText(imMat, "DenseFlow FPS: "+f2str(m_pDF->getFrameRate()), cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	//	putText(imMat, "FlowDepth FPS: "+f2str(m_pDFDepth->getFrameRate()), cv::Point(15,75), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	//	imshow("Depth",*m_pDFDepth->m_pDepth->getCMat());

//	g_pShow->updateFrame(&imMat3);
//	g_pUIMonitor->show();

}

void DroneHunter::handleMouse(int event, int x, int y, int flags)
{
	Rect2d roi;
	int ROIhalf;

	if(m_ROImode == MODE_ASSIST)
	{
		switch (event)
		{
		case EVENT_LBUTTONDOWN:
			if(x > 1820)
			{
				if(y < 100)
				{
					//Clear ROI
					m_pROITracker->tracking(false);
					m_bSelect = false;
					return;
				}
				else if(y < 200)
				{
					//Magnify the ROI size
					if(m_ROIsize < m_ROIsizeTo)
					{
						m_ROIsize += 10;
						if(m_pROITracker->m_bTracking)
						{
							roi.x = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width/2;
							roi.y = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height/2;
							ROIhalf = m_ROIsize/2;

							m_ROI.m_x = roi.x - ROIhalf;
							m_ROI.m_y = roi.y - ROIhalf;
							m_ROI.m_z = roi.x + ROIhalf;
							m_ROI.m_w = roi.y + ROIhalf;
							roi = getROI(m_ROI);
							m_pROITracker->setROI(roi);
							m_bSelect = true;
						}
					}
					m_bSelect = false;
					return;
				}
				else if(y < 300)
				{
					//Shrink the ROI size
					if(m_ROIsize > m_ROIsizeFrom)
					{
						m_ROIsize -= 10;
						if(m_pROITracker->m_bTracking)
						{
							roi.x = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width/2;
							roi.y = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height/2;
							ROIhalf = m_ROIsize/2;

							m_ROI.m_x = roi.x - ROIhalf;
							m_ROI.m_y = roi.y - ROIhalf;
							m_ROI.m_z = roi.x + ROIhalf;
							m_ROI.m_w = roi.y + ROIhalf;
							roi = getROI(m_ROI);
							m_pROITracker->setROI(roi);
							m_bSelect = true;
						}
					}
					m_bSelect = false;
					return;
				}
				else if(y > m_btnMode)
				{
					m_ROI.m_x = 0;
					m_ROI.m_y = 0;
					m_ROI.m_z = 0;
					m_ROI.m_w = 0;
					m_pROITracker->tracking(false);
					m_bSelect = false;
					m_ROImode = MODE_RECTDRAW;
					return;
				}
			}

			ROIhalf = m_ROIsize/2;

			m_ROI.m_x = x - ROIhalf;
			m_ROI.m_y = y - ROIhalf;
			m_ROI.m_z = x + ROIhalf;
			m_ROI.m_w = y + ROIhalf;
			roi = getROI(m_ROI);
			m_pROITracker->setROI(roi);
			m_pROITracker->tracking(true);
			m_bSelect = true;
			break;
		case EVENT_MOUSEMOVE:
			if(m_bSelect)
			{
				ROIhalf = m_ROIsize/2;
				m_ROI.m_x = x - ROIhalf;
				m_ROI.m_y = y - ROIhalf;
				m_ROI.m_z = x + ROIhalf;
				m_ROI.m_w = y + ROIhalf;
				roi = getROI(m_ROI);
				m_pROITracker->setROI(roi);
				m_pROITracker->tracking(true);
			}
			break;
		case EVENT_LBUTTONUP:
			m_bSelect = false;
			break;
		case EVENT_RBUTTONDOWN:
			break;
		default:
			break;
		}

	}
	else if(m_ROImode == MODE_RECTDRAW)
	{
		switch (event)
		{
		case EVENT_LBUTTONDOWN:
			if(x > 1820)
			{
				if(y > m_btnMode)
				{
					m_ROI.m_x = 0;
					m_ROI.m_y = 0;
					m_ROI.m_z = 0;
					m_ROI.m_w = 0;
					m_pROITracker->tracking(false);
					m_bSelect = false;
					m_ROImode = MODE_ASSIST;
					return;
				}
			}

			m_pROITracker->tracking(false);
			m_ROI.m_x = x;
			m_ROI.m_y = y;
			m_ROI.m_z = x;
			m_ROI.m_w = y;
			m_bSelect = true;
			break;
		case EVENT_MOUSEMOVE:
			if(m_bSelect)
			{
				m_ROI.m_z = x;
				m_ROI.m_w = y;
			}
			break;
		case EVENT_LBUTTONUP:
			roi = getROI(m_ROI);
			if(roi.width<m_minROI || roi.height<m_minROI)
			{
				m_ROI.m_x = 0;
				m_ROI.m_y = 0;
				m_ROI.m_z = 0;
				m_ROI.m_w = 0;
			}
			else
			{
				m_pROITracker->setROI(roi);
				m_pROITracker->tracking(true);
			}
			m_bSelect = false;
			break;
		case EVENT_RBUTTONDOWN:
			break;
		default:
			break;
		}
	}

}

Rect2d DroneHunter::getROI(iVector4 mouseROI)
{
	Rect2d roi;

	roi.x = min(mouseROI.m_x,mouseROI.m_z);
	roi.y = min(mouseROI.m_y,mouseROI.m_w);
	roi.width = abs(mouseROI.m_z - mouseROI.m_x);
	roi.height = abs(mouseROI.m_w - mouseROI.m_y);

	return roi;
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



}
