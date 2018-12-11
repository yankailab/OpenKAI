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
	m_appName = "";
	m_bWindow = true;
	m_bDraw = true;
	m_waitKey = 50;
	m_bRun = true;
	m_key = 0;
	m_bConsole = true;
	m_bLog = true;
	m_bStdErr = true;
	m_rc = "";
	m_consoleMsg = "";
	m_consoleMsgLevel = -1;
	m_vInst.clear();
}

Startup::~Startup()
{
	m_vInst.clear();
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
	KISSm(pApp,bConsole);
	KISSm(pApp,bLog);
	KISSm(pApp,bStdErr);
	KISSm(pApp,waitKey);
	KISSm(pApp,rc);

	if(!m_rc.empty())
	{
		system(m_rc.c_str());
	}

	if(!m_bStdErr)
	{
		freopen("/dev/null", "w", stderr);
	}

	FLAGS_logtostderr = 1;
	google::InitGoogleLogging("OpenKAI");
	printEnvironment();

	F_ERROR_F(createAllInst(pKiss));

	unsigned int i;
	for (i = 0; i < m_vInst.size(); i++)
	{
		F_ERROR_F(m_vInst[i].m_pInst->init(m_vInst[i].m_pKiss));
	}

	for (i = 0; i < m_vInst.size(); i++)
	{
		m_vInst[i].m_pInst->start();
	}

	if(m_bConsole)
	{
		initscr();
		noecho();
		cbreak();
		start_color();
		use_default_colors();
		init_pair(CONSOLE_COL_TITLE, COLOR_WHITE, -1);
		init_pair(CONSOLE_COL_NAME, COLOR_GREEN, -1);
		init_pair(CONSOLE_COL_FPS, COLOR_YELLOW, -1);
		init_pair(CONSOLE_COL_MSG, COLOR_WHITE, -1);
		init_pair(CONSOLE_COL_ERROR, COLOR_RED, -1);
	}

	//UI thread
	m_bRun = true;
	unsigned int uWaitKey = m_waitKey * 1000;

	while (m_bRun)
	{
		if(m_bConsole)
		{
			erase();
			console();
			move(0,0);
		    refresh();
		}

		if(m_bDraw)
		{
			draw();
			m_key = waitKey(m_waitKey);
		}
		else
		{
			usleep(uWaitKey);
		}

		if(m_bWindow)
		{
			handleKey(m_key);
		}
	}

	if(m_bConsole)
	{
		endwin();
	}

	for (i = 0; i < m_vInst.size(); i++)
	{
		DEL(m_vInst[i].m_pInst);
	}

	return 0;
}

void Startup::draw(void)
{
	for (unsigned int i = 0; i < m_vInst.size(); i++)
	{
		m_vInst[i].m_pInst->draw();
	}
}

void Startup::console(void)
{
	COL_TITLE;
    mvaddstr(0, 1, m_appName.c_str());
	int iY = 1;

	for (unsigned int i = 0; i < m_vInst.size(); i++)
	{
		m_vInst[i].m_pInst->console(iY);
		iY++;
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

	OK_INST o;
	int i = 0;
	while (pItr[i])
	{
		Kiss* pK = pItr[i++];
		IF_CONT(pK->m_class == "Startup");

		o.m_pInst = m_module.createInstance(pK);
		IF_Fl(!o.m_pInst, "Create instance failed: " + pK->m_name);

		o.m_pKiss = pK;
		m_vInst.push_back(o);

		pK->m_pInst = o.m_pInst;
	}

	return true;
}

void Startup::printEnvironment(void)
{
	LOG_I("OpenCV optimized code:" + i2str(useOptimized()));

#ifdef USE_CUDA
	LOG_I("CUDA devices:" + i2str(cuda::getCudaEnabledDeviceCount()));
	LOG_I("Using CUDA device:" + i2str(cuda::getDevice()));
#endif

	if (ocl::haveOpenCL())
	{
		LOG_I("OpenCL is found");
		ocl::setUseOpenCL(true);
		if (ocl::useOpenCL())
		{
			LOG_I("Using OpenCL");
		}
	}
	else
	{
		LOG_I("OpenCL not found");
	}
}

}
