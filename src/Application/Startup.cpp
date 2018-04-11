#include "Startup.h"

Startup* g_pStartup;

void signalHandler(int signal)
{
	if (signal != SIGINT)return;
	printf("\nSIGINT: OpenKAI Shutdown\n");
	g_pStartup->m_bRun = false;
}

namespace kai
{

Startup::Startup()
{
	for (int i = 0; i < N_INST; i++)
	{
		m_ppInst[i] = NULL;
	}
	m_nInst = 0;

	m_appName = "";
	m_bWindow = true;
	m_bDraw = true;
	m_waitKey = 50;
	m_bRun = true;
	m_key = 0;
	m_bLog = true;
}

Startup::~Startup()
{
}

string* Startup::getName(void)
{
	return &m_appName;
}

bool Startup::start(Kiss* pKiss)
{
	g_pStartup = this;
	signal(SIGINT, signalHandler);

	NULL_F(pKiss);
	Kiss* pApp = pKiss->root()->o("APP");
	IF_F(pApp->empty());

	KISSm(pApp,appName);
	KISSm(pApp,bWindow);
	KISSm(pApp,bDraw);
	KISSm(pApp,waitKey);

	F_FATAL_F(createAllInst(pKiss));

	int i;
	for (i = 0; i < m_nInst; i++)
	{
		F_FATAL_F(m_ppInst[i]->link());
	}

	for (i = 0; i < m_nInst; i++)
	{
		F_FATAL_F(m_ppInst[i]->start());
	}

	//UI thread
	m_bRun = true;

	if (m_bWindow)
	{
		while (m_bRun)
		{
			draw();
			m_key = waitKey(m_waitKey);
			handleKey(m_key);
		}
	}
	else if (m_bDraw)
	{
		while (m_bRun)
		{
			draw();
			waitKey(m_waitKey);
		}
	}
	else
	{
		while (m_bRun)
		{
			sleep(1);
		}
	}

	for (i = 0; i < m_nInst; i++)
	{
		m_ppInst[i]->reset();
	}

	for (i = 0; i < m_nInst; i++)
	{
		DEL(m_ppInst[i]);
	}

	return 0;
}

void Startup::draw(void)
{
	for (int i = 0; i < m_nInst; i++)
	{
		BASE* pInst = m_ppInst[i];
		pInst->draw();
	}
}

void Startup::handleKey(int key)
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

bool Startup::createAllInst(Kiss* pKiss)
{
	NULL_F(pKiss);
	Kiss** pItr = pKiss->root()->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Kiss* pK = pItr[i++];
		if (pK->m_class == "Startup")
			continue;

		BASE* pNew = m_module.createInstance(pK);
		if (pNew == NULL)
		{
			LOG_E("Create instance failed: " + pK->m_name);
			continue;
		}

		if (m_nInst >= N_INST)
		{
			LOG_E("Number of module instances reached limit");
			return false;
		}

		m_ppInst[m_nInst] = pNew;
		m_nInst++;
	}

	return true;
}

}
