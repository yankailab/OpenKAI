#include "General.h"

General* g_pGeneral;
void onMouseGeneral(int event, int x, int y, int flags, void* userdata)
{
	g_pGeneral->handleMouse(event, x, y, flags);
}

namespace kai
{

General::General()
{
	AppBase();

	for (int i = 0; i < N_INST; i++)
	{
		m_pInst[i] = NULL;
	}
	m_nInst = 0;
	m_pFrame = NULL;




	m_pStream1 = NULL;
	m_pAP = NULL;
	m_pMavlink1 = NULL;
	m_pRC1 = NULL;
	m_pCascade = NULL;
	m_pFlow = NULL;
	m_pROITracker = NULL;
	m_pDD = NULL;
	m_pMD = NULL;
	m_pUniverse = NULL;
	m_pFCN = NULL;
	m_pSSD = NULL;
	m_pAT = NULL;
	m_pAM1 = NULL;
}

General::~General()
{
}

bool General::start(Config* pConfig)
{
	//TODO: Caffe set data to GPU directly
	//TODO: Solve caffe ROI in DepthDetector
	//TODO: Optimize FCN

	NULL_F(pConfig);

	g_pGeneral = this;
	Config* pApp = pConfig->o("APP");
	if(pApp->empty())return false;

	F_INFO_(pApp->v("appName", &m_name));
	F_INFO_(pApp->v("bShowScreen", &m_bShowScreen));
	F_INFO_(pApp->v("bFullScreen", &m_bFullScreen));
	F_INFO_(pApp->v("waitKey", &m_waitKey));


	F_FATAL_F(createAllInst(pConfig));






/*
	//temporal
	string cName;

	//Init Camera
	int FPS;
	string camName;
	F_INFO_(pConfig->o("APP")->v("camMain", &camName));
	if (camName != "video")
	{
		m_pStream1 = new _Stream();
		F_FATAL_F(m_pStream1->init(pConfig, &camName));
		m_pStream1->start();
	}

	//Init Automaton
	FPS = 0;
	cName = "Automaton0";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pAM1 = new _Automaton();
		F_FATAL_F(m_pAM1->init(pConfig, &cName));
		m_pAM1->start();
	}

	//Init ROI Tracker
	FPS = 0;
	cName = "roiTracker";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pROITracker = new _ROITracker();
		m_pROITracker->m_pCamStream = m_pStream1;
		m_pROITracker->init(pConfig, &cName);
		m_pROITracker->start();
	}

	//Init Marker Detector
	FPS = 0;
	cName = "markerLanding";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pMD = new _Bullseye();
		m_pStream1->m_bGray = true;
		m_pStream1->m_bHSV = true;
		m_pMD->m_pCamStream = m_pStream1;
		m_pMD->init(pConfig, &cName);
		m_pMD->start();
	}

	//Init AprilTags Detector
	FPS = 0;
	cName = "AprilTagLanding";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pAT = new _AprilTags();
		m_pAT->m_pCamStream = m_pStream1;
		m_pAT->init(pConfig, &cName);
		m_pAT->start();
	}

	//Init Mavlink
	FPS = 0;
	cName = "Mavlink0";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pMavlink1 = new _Mavlink();
		F_FATAL_F(m_pMavlink1->init(pConfig, &cName));
		m_pMavlink1->start();
	}

	//Connect to RC output
	FPS = 0;
	cName = "RC_0";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pRC1 = new _RC();
		F_FATAL_F(m_pRC1->init(pConfig, &cName));
		F_FATAL_F(m_pRC1->open());
		m_pRC1->start();
	}

	//Init Autopilot
	FPS = 0;
	cName = "Autopilot0";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pAP = new _AutoPilot();
		m_pAP->m_pMavlink = m_pMavlink1;
//		m_pAP->m_pROITracker = m_pROITracker;
//		m_pAP->m_pMD = m_pMD;
//		m_pAP->m_pAT = m_pAT;
		m_pAP->m_pAM = m_pAM1;
		F_FATAL_F(m_pAP->init(pConfig, &cName));
		m_pAP->start();
	}

	//Init Universe
	FPS = 0;
	cName = "Universe";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pUniverse = new _Universe();
		m_pUniverse->init(pConfig, &cName);
		m_pUniverse->start();
	}

	//Init SSD
	FPS = 0;
	cName = "SSD";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pSSD = new _SSD();
		m_pSSD->m_pCamStream = m_pStream1;
		m_pSSD->m_pUniverse = m_pUniverse;
		m_pSSD->init(pConfig, &cName);
		m_pSSD->start();
	}

	//Init Optical Flow
	FPS = 0;
	cName = "optflow0";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pFlow = new _Flow();
		m_pFlow->m_pCamStream = m_pStream1;
		m_pStream1->m_bGray = true;
		F_FATAL_F(m_pFlow->init(pConfig, &cName));
		m_pFlow->start();
	}

	//Init Depth Object Detector
	FPS = 0;
	cName = "depthObjDetector";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pDD = new _Depth();
		m_pDD->m_pCamStream = m_pStream1;
		m_pDD->m_pUniverse = m_pUniverse;
		m_pDD->m_pFlow = m_pFlow;
		F_FATAL_F(m_pDD->init(pConfig, &cName));
		m_pDD->start();
	}

	//Init FCN
	FPS = 0;
	cName = "FCN";
	F_INFO_(pConfig->o(cName)->v("FPS", &FPS));
	if (FPS > 0)
	{
		m_pFCN = new _FCN();
		m_pFCN->m_pCamStream = m_pStream1;
		m_pFCN->init(pConfig, &cName);
		m_pFCN->start();
	}
*/




	//UI thread
	m_bRun = true;

	if (m_bShowScreen)
	{
		m_pFrame = new Frame();

		namedWindow(m_name, CV_WINDOW_NORMAL);
		if (m_bFullScreen)
		{
			setWindowProperty(m_name, CV_WND_PROP_FULLSCREEN,
					CV_WINDOW_FULLSCREEN);
		}
		setMouseCallback(m_name, onMouseGeneral, NULL);

		while (m_bRun)
		{
	//		Mavlink_Messages mMsg;
	//		mMsg = m_pMavlink->current_messages;
	//		m_pCamFront->m_pCamL->m_bGimbal = true;
	//		m_pCamFront->m_pCamL->setAttitude(mMsg.attitude.roll, 0, mMsg.time_stamps.attitude);

			draw();

			//Handle key input
			//TODO: move into bShowScreen
			m_key = waitKey(m_waitKey);
			handleKey(m_key);
		}

//		STOP(m_pAP);
//		STOP(m_pMavlink1);
//		STOP(m_pROITracker);
//		STOP(m_pMD);
//		STOP(m_pUniverse);
//		STOP(m_pDD);
//		STOP(m_pFlow);
//		STOP(m_pFCN);
	}

/*

//	COMPLETE(m_pAP);
	COMPLETE(m_pROITracker);
	COMPLETE(m_pMD);
	COMPLETE(m_pUniverse);
	COMPLETE(m_pDD);
	COMPLETE(m_pFlow);
	COMPLETE(m_pFCN);
	COMPLETE(m_pStream1);

	m_pMavlink1->complete();
	m_pMavlink1->close();

	RELEASE(m_pMavlink1);
	RELEASE(m_pStream);
	RELEASE(m_pROITracker);
	RELEASE(m_pAP);
	RELEASE(m_pMD);
	RELEASE(m_pUniverse);
	RELEASE(m_pDD);
	RELEASE(m_pFlow);
	RELEASE(m_pFCN);
*/
	return 0;

}

void General::draw(void)
{
	NULL_(m_pFrame);

	iVec4 textPos;
	textPos.m_x = 15;
	textPos.m_y = 20;
	textPos.m_w = 20;
	textPos.m_z = 500;

	if (m_pStream1)
	{
		if (!m_pStream1->draw(m_pFrame, &textPos))
			return;
	}

	if (m_pAM1)
	{
		m_pAM1->draw(m_pFrame, &textPos);
	}

//	if(m_pAP)
//	{
//		m_pAP->draw(m_pFrame, &textPos);
//	}

	if (m_pAT)
	{
		m_pAT->draw(m_pFrame, &textPos);
	}

	if (m_pUniverse)
	{
		m_pUniverse->draw(m_pFrame, &textPos);
	}

	if (m_pSSD)
	{
		m_pSSD->draw(m_pFrame, &textPos);
	}

	if (m_pMavlink1)
	{
		m_pMavlink1->draw(m_pFrame, &textPos);
	}

	if (m_pFCN)
	{
		m_pFCN->draw(m_pFrame, &textPos);
	}

	imshow(m_name, *m_pFrame->getCMat());

}

void General::handleKey(int key)
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

void General::handleMouse(int event, int x, int y, int flags)
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


bool General::createAllInst(Config* pConfig)
{
	NULL_F(pConfig);
	Config** pItr = pConfig->root()->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Config* pC = pItr[i++];

		bool bInst = false;
		F_INFO_(pC->v("bInst", &bInst));
		if (!bInst)continue;


		if (pC->m_class == "_Stream")
		{
			F_FATAL_F(createInst<_Stream>(pConfig));
		}
		else if (pC->m_class == "_Automaton")
		{
			F_FATAL_F(createInst<_Automaton>(pConfig));
		}
		else if (pC->m_class == "_Universe")
		{
			F_FATAL_F(createInst<_Universe>(pConfig));
		}
		else if (pC->m_class == "_Mavlink")
		{
			F_FATAL_F(createInst<_Mavlink>(pConfig));
		}
		else if (pC->m_class == "_RC")
		{
			F_FATAL_F(createInst<_RC>(pConfig));
		}
		else if (pC->m_class == "_SSD")
		{
			F_FATAL_F(createInst<_SSD>(pConfig));
		}
		else if (pC->m_class == "_FCN")
		{
			F_FATAL_F(createInst<_FCN>(pConfig));
		}
		else if (pC->m_class == "_AutoPilot")
		{
			F_FATAL_F(createInst<_AutoPilot>(pConfig));
		}
		else if (pC->m_class == "_AprilTags")
		{
			F_FATAL_F(createInst<_AprilTags>(pConfig));
		}
		else if (pC->m_class == "_Bullseye")
		{
			F_FATAL_F(createInst<_Bullseye>(pConfig));
		}
		else if (pC->m_class == "_ROITracker")
		{
			F_FATAL_F(createInst<_ROITracker>(pConfig));
		}
		else if (pC->m_class == "_Cascade")
		{
			F_FATAL_F(createInst<_Cascade>(pConfig));
		}
		else if (pC->m_class == "_Depth")
		{
			F_FATAL_F(createInst<_Depth>(pConfig));
		}
		else if (pC->m_class == "_Flow")
		{
			F_FATAL_F(createInst<_Flow>(pConfig));
		}
		else if (pC->m_class == "UI")
		{
			F_FATAL_F(createInst<UI>(pConfig));
		}
		else if(pC->m_class != "General")
		{
			LOG(ERROR)<<"Unknown class type";
		}

	}

	return true;
}

template <typename T> bool General::createInst(Config* pConfig)
{
	if(m_nInst>=N_INST)return false;
	NULL_F(pConfig);

	T* pInst = new T();
	F_FATAL_F(pInst->T::init(pConfig));
	pConfig->m_pInst = pInst;

	m_pInst[m_nInst] = pInst;
	m_nInst++;

//	F_FATAL_F(pInst->T::start());

    return true;
}

}
