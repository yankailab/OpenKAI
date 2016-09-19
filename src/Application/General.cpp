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

	//create instances and link with each other
	F_FATAL_F(createAllInst(pConfig));

	//UI thread
	m_bRun = true;

	if (m_bShowScreen)
	{
		m_pFrame = new Frame();

		namedWindow(m_name, CV_WINDOW_NORMAL);
		if (m_bFullScreen)
		{
			setWindowProperty(m_name,
					CV_WND_PROP_FULLSCREEN,
					CV_WINDOW_FULLSCREEN);
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

	iVec4 textPos;
	textPos.m_x = 15;
	textPos.m_y = 20;
	textPos.m_w = 20;
	textPos.m_z = 500;

	for(int i=0; i<m_nInst; i++)
	{
		BASE* pInst = m_pInst[i];
		string* cName = pInst->getClass();

		if (*cName == "_Stream")
		{
			F_(((_Stream*)pInst)->draw(m_pFrame, &textPos));
		}
		else if (*cName == "_Automaton")
		{
			F_(((_Automaton*)pInst)->draw(m_pFrame, &textPos));
		}
		else if (*cName == "_Universe")
		{
			F_(((_Universe*)pInst)->draw(m_pFrame, &textPos));
		}
		else if (*cName == "_Mavlink")
		{
			F_(((_Mavlink*)pInst)->draw(m_pFrame, &textPos));
		}
		else if (*cName == "_SSD")
		{
			F_(((_SSD*)pInst)->draw(m_pFrame, &textPos));
		}
		else if (*cName == "_FCN")
		{
			F_(((_FCN*)pInst)->draw(m_pFrame, &textPos));
		}
		else if (*cName == "_AutoPilot")
		{
			F_(((_AutoPilot*)pInst)->draw(m_pFrame, &textPos));
		}
		else if (*cName == "_AprilTags")
		{
			F_(((_AprilTags*)pInst)->draw(m_pFrame, &textPos));
		}
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
		else if (pC->m_class == "_RC")
		{
			F_FATAL_F(createInst<_RC>(pC));
		}
		else if (pC->m_class == "_SSD")
		{
			F_FATAL_F(createInst<_SSD>(pC));
		}
		else if (pC->m_class == "_FCN")
		{
			F_FATAL_F(createInst<_FCN>(pC));
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
		else if (pC->m_class == "_Cascade")
		{
			F_FATAL_F(createInst<_Cascade>(pC));
		}
		else if (pC->m_class == "_Depth")
		{
			F_FATAL_F(createInst<_Depth>(pC));
		}
		else if (pC->m_class == "_Flow")
		{
			F_FATAL_F(createInst<_Flow>(pC));
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

	F_FATAL_F(pInst->T::start());
    return true;
}

}
