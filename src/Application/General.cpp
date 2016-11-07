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
		m_pMouse[i] = NULL;
	}
	m_nInst = 0;
	m_nMouse = 0;
	m_pFrame = NULL;

}

General::~General()
{
}

bool General::start(Config* pConfig)
{
	//TODO: Caffe set data to GPU directly
	//TODO: Solve caffe ROI in DepthDetector
	//TODO: Optimize FCN

	int i;
	NULL_F(pConfig);

	g_pGeneral = this;
	Config* pApp = pConfig->root()->o("APP");
	if(pApp->empty())return false;

	F_INFO(pApp->v("appName", &m_name));
	F_INFO(pApp->v("bShowScreen", &m_bShowScreen));
	F_INFO(pApp->v("bFullScreen", &m_bFullScreen));
	F_INFO(pApp->v("waitKey", &m_waitKey));

	//create instances
	F_FATAL_F(createAllInst(pConfig));

	//link instances with each other
	F_FATAL_F(linkAllInst(pConfig));

	//UI thread
	m_bRun = true;

	if (m_bShowScreen)
	{
		m_pFrame = new Frame();

		if (m_bFullScreen)
		{
			namedWindow(m_name, CV_WINDOW_NORMAL);
			setWindowProperty(m_name,
					CV_WND_PROP_FULLSCREEN,
					CV_WINDOW_FULLSCREEN);
		}
		else
		{
			namedWindow(m_name, CV_WINDOW_AUTOSIZE);
		}
		setMouseCallback(m_name, onMouseGeneral, NULL);

		while (m_bRun)
		{
			draw();

			m_key = waitKey(m_waitKey);
			handleKey(m_key);
		}

		for(i=0;i<m_nInst;i++)
		{
			((_ThreadBase*)m_pInst[i])->complete();
		}
	}

	for(i=0;i<m_nInst;i++)
	{
		((_ThreadBase*)m_pInst[i])->waitForComplete();
	}

	for(i=0;i<m_nInst;i++)
	{
		DEL(m_pInst[i]);
	}

	return 0;

}

void General::draw(void)
{
	NULL_(m_pFrame);

	m_pFrame->allocate(1280,1280);

	iVec4 textPos;
	textPos.m_x = 15;
	textPos.m_y = 20;
	textPos.m_w = 20;
	textPos.m_z = 500;

	for(int i=0; i<m_nInst; i++)
	{
		BASE* pInst = m_pInst[i];
		string* cName = pInst->getClass();

		//Make sure the stream class comes to the first in config file as stream will format the Mat with camera stream
		if (*cName == "_Stream")
		{
			if(!((_Stream*)pInst)->draw(m_pFrame, &textPos))return;
		}
		else if (*cName == "_Automaton")
		{
			((_Automaton*)pInst)->draw(m_pFrame, &textPos);
		}
		else if (*cName == "_Universe")
		{
			((_Universe*)pInst)->draw(m_pFrame, &textPos);
		}
		else if (*cName == "_Mavlink")
		{
			((_Mavlink*)pInst)->draw(m_pFrame, &textPos);
		}
		else if (*cName == "_Canbus")
		{
			((_Canbus*)pInst)->draw(m_pFrame, &textPos);
		}
		else if (*cName == "_AutoPilot")
		{
			((_AutoPilot*)pInst)->draw(m_pFrame, &textPos);
		}
		else if (*cName == "_AprilTags")
		{
			((_AprilTags*)pInst)->draw(m_pFrame, &textPos);
		}
		else if (*cName == "_Lightware_SF40")
		{
			((_Lightware_SF40*)pInst)->draw(m_pFrame, &textPos);
		}
#ifdef USE_SSD
		else if (*cName == "_SSD")
		{
			((_SSD*)pInst)->draw(m_pFrame, &textPos);
		}
#endif
#ifdef USE_FCN
		else if (*cName == "_FCN")
		{
			((_FCN*)pInst)->draw(m_pFrame, &textPos);
		}
#endif
#ifdef USE_TENSORRT
		else if (*cName == "_DetectNet")
		{
			((_DetectNet*)pInst)->draw(m_pFrame, &textPos);
		}
#endif
//		else if (*cName == "_RC")
//		{
//			F_(((_RC*)pInst)->draw(m_pFrame, &textPos));
//		}
//		else if (*cName == "_ROITracker")
//		{
//			F_(((_ROITracker*)pInst)->draw(m_pFrame, &textPos));
//		}
//		else if (*cName == "_Cascade")
//		{
//			F_(((_Cascade*)pInst)->draw(m_pFrame, &textPos));
//		}
//		else if (*cName == "_Bullseye")
//		{
//			F_(((_Bullseye*)pInst)->draw(m_pFrame, &textPos));
//		}
//		else if (*cName == "_Depth")
//		{
//			F_(((_Depth*)pInst)->draw(m_pFrame, &textPos));
//		}
//		else if (*cName == "_Flow")
//		{
//			F_(((_Flow*)pInst)->draw(m_pFrame, &textPos));
//		}
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
	MOUSE m;
	m.m_event = event;
	m.m_flags = flags;
	m.m_x = x;
	m.m_y = y;

	int i;
	for(i=0;i<m_nMouse;i++)
	{
		((_AutoPilot*)m_pMouse[i])->onMouse(&m);
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
		F_INFO(pC->v("bInst", &bInst));
		if (!bInst)continue;


		if (pC->m_class == "_Stream")
		{
			F_FATAL_F(createInst<_Stream>(pC));
		}
		else if (pC->m_class == "_Automaton")
		{
			F_FATAL_F(createInst<_Automaton>(pC));
		}
		else if (pC->m_class == "_Universe")
		{
			F_FATAL_F(createInst<_Universe>(pC));
		}
		else if (pC->m_class == "_Mavlink")
		{
			F_FATAL_F(createInst<_Mavlink>(pC));
		}
		else if (pC->m_class == "_Canbus")
		{
			F_FATAL_F(createInst<_Canbus>(pC));
		}
		else if (pC->m_class == "_RC")
		{
			F_FATAL_F(createInst<_RC>(pC));
		}
		else if (pC->m_class == "_AutoPilot")
		{
			F_FATAL_F(createInst<_AutoPilot>(pC));
		}
		else if (pC->m_class == "_AprilTags")
		{
			F_FATAL_F(createInst<_AprilTags>(pC));
		}
		else if (pC->m_class == "_Bullseye")
		{
			F_FATAL_F(createInst<_Bullseye>(pC));
		}
		else if (pC->m_class == "_ROITracker")
		{
			F_FATAL_F(createInst<_ROITracker>(pC));
		}
		else if (pC->m_class == "_Flow")
		{
			F_FATAL_F(createInst<_Flow>(pC));
		}
		else if (pC->m_class == "_Depth")
		{
			F_FATAL_F(createInst<_Depth>(pC));
		}
		else if (pC->m_class == "_Lightware_SF40")
		{
			F_FATAL_F(createInst<_Lightware_SF40>(pC));
		}
#ifdef USE_SSD
		else if (pC->m_class == "_SSD")
		{
			F_FATAL_F(createInst<_SSD>(pC));
		}
#endif
#ifdef USE_FCN
		else if (pC->m_class == "_FCN")
		{
			F_FATAL_F(createInst<_FCN>(pC));
		}
#endif
#ifdef USE_TENSORRT
		else if (pC->m_class == "_DetectNet")
		{
			F_FATAL_F(createInst<_DetectNet>(pC));
		}
#endif
		else if(pC->m_class != "General")
		{
			LOG(ERROR)<<"Unknown class";
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

	m_pInst[m_nInst] = pInst;
	m_nInst++;

	F_FATAL_F(pInst->T::start());

	CHECK_T(pConfig->m_class != "_AutoPilot");
	CHECK_T(!m_bShowScreen);

	m_pMouse[m_nMouse] = pInst;
	m_nMouse++;

    return true;
}

bool General::linkAllInst(Config* pConfig)
{
	NULL_F(pConfig);
	Config** pItr = pConfig->root()->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Config* pC = pItr[i++];

		bool bInst = false;
		F_INFO(pC->v("bInst", &bInst));
		if (!bInst)continue;

		if (pC->m_class == "_Stream")
		{
			F_FATAL_F(((_Stream*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_Automaton")
		{
			F_FATAL_F(((_Automaton*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_Universe")
		{
			F_FATAL_F(((_Universe*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_Mavlink")
		{
			F_FATAL_F(((_Mavlink*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_Canbus")
		{
			F_FATAL_F(((_Canbus*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_RC")
		{
			F_FATAL_F(((_RC*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_AutoPilot")
		{
			F_FATAL_F(((_AutoPilot*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_AprilTags")
		{
			F_FATAL_F(((_AprilTags*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_Bullseye")
		{
			F_FATAL_F(((_Bullseye*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_ROITracker")
		{
			F_FATAL_F(((_ROITracker*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_Depth")
		{
			F_FATAL_F(((_Depth*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_Flow")
		{
			F_FATAL_F(((_Flow*)(pC->m_pInst))->link());
		}
		else if (pC->m_class == "_Lightware_SF40")
		{
			F_FATAL_F(((_Lightware_SF40*)(pC->m_pInst))->link());
		}
#ifdef USE_SSD
		else if (pC->m_class == "_SSD")
		{
			F_FATAL_F(((_SSD*)(pC->m_pInst))->link());
		}
#endif
#ifdef USE_FCN
		else if (pC->m_class == "_FCN")
		{
			F_FATAL_F(((_FCN*)(pC->m_pInst))->link());
		}
#endif
#ifdef USE_TENSORRT
		else if (pC->m_class == "_DetectNet")
		{
			F_FATAL_F(((_DetectNet*)(pC->m_pInst))->link());
		}
#endif
		else if(pC->m_class != "General")
		{
			LOG(ERROR)<<"Unknown class";
		}

	}

	return true;
}


}
