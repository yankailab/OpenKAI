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
	Config* pApp = pConfig->o("APP");
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
			drawInst<_Stream>((_Stream*)pInst, m_pFrame, &textPos);
		}
		else if (*cName == "_Automaton")
		{
			drawInst<_Automaton>((_Automaton*)pInst, m_pFrame, &textPos);
		}
		else if (*cName == "_Universe")
		{
			drawInst<_Universe>((_Universe*)pInst, m_pFrame, &textPos);
		}
		else if (*cName == "_Mavlink")
		{
			drawInst<_Mavlink>((_Mavlink*)pInst, m_pFrame, &textPos);
		}
		else if (*cName == "_SSD")
		{
			drawInst<_SSD>((_SSD*)pInst, m_pFrame, &textPos);
		}
		else if (*cName == "_FCN")
		{
			drawInst<_FCN>((_FCN*)pInst, m_pFrame, &textPos);
		}
		else if (*cName == "_AutoPilot")
		{
			drawInst<_AutoPilot>((_AutoPilot*)pInst, m_pFrame, &textPos);
		}
		else if (*cName == "_AprilTags")
		{
			drawInst<_AprilTags>((_AprilTags*)pInst, m_pFrame, &textPos);
		}
//		else if (*cName == "_RC")
//		{
//			drawInst<_RC>((_RC*)pInst, m_pFrame, &textPos);
//		}
//		else if (*cName == "_ROITracker")
//		{
//			drawInst<_ROITracker>((_ROITracker*)pInst, m_pFrame, &textPos);
//		}
//		else if (*cName == "_Cascade")
//		{
//			drawInst<_Cascade>((_Cascade*)pInst, m_pFrame, &textPos);
//		}
//		else if (*cName == "_Bullseye")
//		{
//			drawInst<_Bullseye>((_Bullseye*)pInst, m_pFrame, &textPos);
//		}
//		else if (*cName == "_Depth")
//		{
//			drawInst<_Depth>((_Depth*)pInst, m_pFrame, &textPos);
//		}
//		else if (*cName == "_Flow")
//		{
//			drawInst<_Flow>((_Flow*)pInst, m_pFrame, &textPos);
//		}
	}

	imshow(m_name, *m_pFrame->getCMat());
}

template <typename T> void General::drawInst(T* pInst, Frame* pFrame, iVec4* textPos)
{
	NULL_(pInst);

	((T*)pInst)->draw(pFrame, textPos);
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
