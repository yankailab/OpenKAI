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

bool General::start(Kiss* pKiss)
{
	int i;
	NULL_F(pKiss);

	g_pGeneral = this;
	Kiss* pApp = pKiss->root()->o("APP");
	if(pApp->empty())return false;

	F_INFO(pApp->v("appName", &m_name));
	F_INFO(pApp->v("bShowScreen", &m_bShowScreen));
	F_INFO(pApp->v("bFullScreen", &m_bFullScreen));
	F_INFO(pApp->v("waitKey", &m_waitKey));

	//create instances
	F_FATAL_F(createAllInst(pKiss));

	//link instances with each other
	for(i=0; i<m_nInst; i++)
	{
		F_FATAL_F(m_pInst[i]->link());
	}

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

	vInt4 textPos;
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
			if(!pInst->draw(m_pFrame, &textPos))return;
		}
		else
		{
			pInst->draw(m_pFrame, &textPos);
		}
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

bool General::createAllInst(Kiss* pKiss)
{
	NULL_F(pKiss);
	Kiss** pItr = pKiss->root()->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Kiss* pK = pItr[i++];

		bool bInst = false;
		F_INFO(pK->v("bInst", &bInst));
		if (!bInst)continue;

		if(pK->m_class == "General")continue;

		BASE* pNew = m_module.createInstance(pK);
		if(pNew==NULL)
		{
			LOG(ERROR)<<"Unknown class:"+pK->m_class;
			continue;
		}

		if(m_nInst>=N_INST)
		{
			LOG(ERROR)<<"Number of module instances reached limit";
			return false;
		}

		m_pInst[m_nInst] = pNew;
		m_nInst++;

		F_FATAL_F(pNew->start());

		if(*pNew->getClass() != "_AutoPilot")continue;
		if(!m_bShowScreen)continue;

		m_pMouse[m_nMouse] = pNew;
		m_nMouse++;
	}

	return true;
}

}
