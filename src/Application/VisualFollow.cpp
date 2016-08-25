#include "VisualFollow.h"

VisualFollow* g_pDroneHunter;
void onMouseDroneHunter(int event, int x, int y, int flags, void* userdata)
{
	g_pDroneHunter->handleMouse(event,x,y,flags);
}


namespace kai
{

VisualFollow::VisualFollow()
{
	AppBase();

	m_pCamFront = NULL;
	m_pAP = NULL;
	m_pMavlink = NULL;
	m_pVlink = NULL;
	m_pROITracker = NULL;
	m_pFrame = NULL;

}

VisualFollow::~VisualFollow()
{
}


bool VisualFollow::start(JSON* pJson)
{
	g_pDroneHunter = this;
	int FPS;

	CHECK_INFO(pJson->getVal("APP_SHOW_SCREEN", &m_bShowScreen));
	CHECK_INFO(pJson->getVal("APP_FULL_SCREEN", &m_bFullScreen));
	CHECK_INFO(pJson->getVal("APP_WAIT_KEY", &m_waitKey));

	CHECK_ERROR(pJson->getVal("ROI_MIN_SIZE", &m_minROI));

	m_pFrame = new Frame();

	//Init Camera
	FPS=0;
	CHECK_INFO(pJson->getVal("CAM_FRONTL_FPS", &FPS));
	if (FPS > 0)
	{
		m_pCamFront = new _Stream();
		CHECK_FATAL(m_pCamFront->init(pJson, "FRONTL"));
		m_pCamFront->start();
	}

	//Init ROI Tracker
	FPS=0;
	CHECK_INFO(pJson->getVal("ROITRACKER_MAIN_FPS", &FPS));
	if (FPS > 0)
	{
		m_pROITracker = new _ROITracker();
		m_pROITracker->init(pJson, "MAIN");
		m_pROITracker->m_pCamStream = m_pCamFront;
		m_pROITracker->start();
	}

	//Connect to VehicleLink
	FPS=0;
	CHECK_INFO(pJson->getVal("SERIALPORT_MAVLINK_FPS", &FPS));
	if (FPS > 0)
	{
		m_pVlink = new _RC();
		CHECK_FATAL(m_pVlink->setup(pJson, "MAVLINK"));
		CHECK_INFO(m_pVlink->open());
		m_pVlink->start();
	}

	//Init Autopilot
	FPS=0;
	CHECK_INFO(pJson->getVal("AUTOPILOT_MAIN_FPS", &FPS));
	if (FPS > 0)
	{
		m_pAP = new _AutoPilot();
		CHECK_FATAL(m_pAP->init(pJson, "MAIN"));
		m_pAP->m_pVI = m_pVlink;
		m_pAP->m_pROITracker = m_pROITracker;
		m_pAP->start();
	}

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


	//UI thread
	m_bRun = true;
	namedWindow(APP_NAME, CV_WINDOW_NORMAL);
	setWindowProperty(APP_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	setMouseCallback(APP_NAME, onMouseDroneHunter, NULL);

	while (m_bRun)
	{
		if (m_bShowScreen)
		{
			draw();
		}

		//Handle key input
		m_key = waitKey(m_waitKey);
		handleKey(m_key);
	}

	STOP(m_pAP);
	STOP(m_pVlink);
	STOP(m_pROITracker);

	m_pVlink->complete();
	m_pVlink->close();

	COMPLETE(m_pAP);
	COMPLETE(m_pROITracker);
	COMPLETE(m_pCamFront);

	RELEASE(m_pVlink);
	RELEASE(m_pCamFront);
	RELEASE(m_pROITracker);
	RELEASE(m_pAP);

	return 0;

}

void VisualFollow::draw(void)
{
	Mat imMat;
	Rect2d roi;

	iVector4 textPos;
	textPos.m_x = 15;
	textPos.m_y = 20;
	textPos.m_w = 20;
	textPos.m_z = 500;

	if(m_pCamFront)
	{
		if(!m_pCamFront->draw(m_pFrame, &textPos))return;
	}

	if(m_pAP)
	{
		m_pAP->draw(m_pFrame, &textPos);
	}

	imMat = *m_pFrame->getCMat();

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

	putText(imMat, "ROITracker FPS: "+f2str(m_pROITracker->getFrameRate()), cv::Point(textPos.m_x, textPos.m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	textPos.m_y += textPos.m_w;
	putText(imMat, "AutoPilot FPS: "+f2str(m_pAP->getFrameRate()), cv::Point(textPos.m_x, textPos.m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	textPos.m_y += textPos.m_w;
	putText(imMat, "Roll: "+f2str(m_pAP->m_RC[0]), cv::Point(textPos.m_x, textPos.m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	textPos.m_y += textPos.m_w;
	putText(imMat, "Pitch: "+f2str(m_pAP->m_RC[1]), cv::Point(textPos.m_x, textPos.m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	textPos.m_y += textPos.m_w;

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

}

void VisualFollow::handleMouse(int event, int x, int y, int flags)
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

Rect2d VisualFollow::getROI(iVector4 mouseROI)
{
	Rect2d roi;

	roi.x = min(mouseROI.m_x,mouseROI.m_z);
	roi.y = min(mouseROI.m_y,mouseROI.m_w);
	roi.width = abs(mouseROI.m_z - mouseROI.m_x);
	roi.height = abs(mouseROI.m_w - mouseROI.m_y);

	return roi;
}


void VisualFollow::handleKey(int key)
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



}
