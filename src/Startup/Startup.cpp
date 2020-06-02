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
	m_bLog = true;
	m_bStdErr = true;
	m_rc = "";
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
	Kiss* pApp = pKiss->root()->child("APP");
	IF_F(pApp->empty());
	pApp->m_pInst = (BASE*)this;

	pApp->v("appName", &m_appName);
	pApp->v("bWindow", &m_bWindow);
	pApp->v("bDraw", &m_bDraw);
	pApp->v("bLog", &m_bLog);
	pApp->v("bStdErr", &m_bStdErr);
	pApp->v("waitKey", &m_waitKey);
	pApp->v("rc", &m_rc);

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

	//UI thread
	m_bRun = true;
	unsigned int uWaitKey = m_waitKey * 1000;

	while (m_bRun)
	{
		if(m_bDraw)
		{
			for (unsigned int i = 0; i < m_vInst.size(); i++)
			{
				m_vInst[i].m_pInst->draw();
			}

#ifdef USE_OPENCV
			m_key = waitKey(m_waitKey);
#endif
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

	for (i = 0; i < m_vInst.size(); i++)
	{
		DEL(m_vInst[i].m_pInst);
	}

	return 0;
}

bool Startup::createAllInst(Kiss* pKiss)
{
	NULL_F(pKiss);

	OK_INST o;
	int i = 0;
	while (1)
	{
		Kiss* pK = pKiss->root()->child(i++);
		if(pK->empty())break;

		IF_CONT(pK->m_class == "Startup");

		o.m_pInst = m_module.createInstance(pK);
		if(!o.m_pInst)
		{
			LOG_E("Failed to create instance: " + pK->m_name);
			return false;
		}

		o.m_pKiss = pK;
		m_vInst.push_back(o);

		pK->m_pInst = o.m_pInst;
	}

	return true;
}

void Startup::handleKey(int key)
{
	if(key==27)	//ESC
	{
		m_bRun = false;
		return;
	}

	for(int i=0; i<m_vKeyCallback.size(); i++)
	{
		KEY_CALLBACK* pCB = &m_vKeyCallback[i];
		pCB->m_cbKey(key, pCB->m_pInst);
	}
}

bool Startup::addKeyCallback(CallbackKey cbKey, void* pInst)
{
	NULL_F(cbKey);

	KEY_CALLBACK cb;
	cb.m_cbKey = cbKey;
	cb.m_pInst = pInst;
	m_vKeyCallback.push_back(cb);

	return true;
}

void Startup::printEnvironment(void)
{
#ifdef USE_OPENCV
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
#endif
}

}
