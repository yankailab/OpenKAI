#include "Startup.h"

void signalHandler(int signal)
{
	if (signal != SIGINT)
		return;
	printf("\nSIGINT: OpenKAI Shutdown\n");
	g_pStartup->m_bRun = false;
}

namespace kai
{

	Startup::Startup()
	{
		m_appName = "";
		m_bRun = true;
		m_bStdErr = true;
		m_bLog = false;
		m_rc = "";
		m_vInst.clear();
	}

	Startup::~Startup()
	{
		m_vInst.clear();
	}

	string *Startup::getName(void)
	{
		return &m_appName;
	}

	bool Startup::start(const string &fName)
	{
		IF_F(fName.empty());

		Kiss *pKiss = new Kiss();
		if (!parseKiss(fName, pKiss))
		{
			printf("Kiss file not found or parsing failed\n");
			delete pKiss;
			return false;
		}

		g_pStartup = this;
		signal(SIGINT, signalHandler);

		Kiss *pApp = pKiss->root()->child("APP");
		IF_F(pApp->empty());
		pApp->m_pInst = (BASE *)this;

		pApp->v("appName", &m_appName);
		pApp->v("bStdErr", &m_bStdErr);
		pApp->v("bLog", &m_bLog);
		pApp->v("rc", &m_rc);

		// parse & attach included kiss files
		vector<string> vInclude;
		pApp->a("vInclude", &vInclude);
		for (string s : vInclude)
		{
			if(!parseKiss(s, pKiss))
			{
				LOG(INFO) << "Kiss parse failed: " << s;
			}
		}

		// run cmd
		if (!m_rc.empty())
		{
			system(m_rc.c_str());
		}

		if (!m_bStdErr)
		{
			freopen("/dev/null", "w", stderr);
		}

#ifdef USE_GLOG
		FLAGS_logtostderr = 1;
		google::InitGoogleLogging("OpenKAI");
#endif
		printEnvironment();

		F_ERROR_F(createAllInst(pKiss));
		unsigned int i;

		for (i = 0; i < m_vInst.size(); i++)
		{
			IF_CONT(m_vInst[i].m_pInst->init(m_vInst[i].m_pKiss));
			LOG_E(m_vInst[i].m_pKiss->m_name + ".init()");
			return false;
		}

		for (i = 0; i < m_vInst.size(); i++)
		{
			IF_CONT(m_vInst[i].m_pInst->link());
			LOG_E(m_vInst[i].m_pKiss->m_name + ".link()");
			return false;
		}

		for (i = 0; i < m_vInst.size(); i++)
		{
			IF_CONT(m_vInst[i].m_pInst->start());
			LOG_E(m_vInst[i].m_pKiss->m_name + ".start()");
			return false;
		}

		// UI thread
		m_bRun = true;
		while (m_bRun)
		{
			sleep(1);
		}

		for (i = 0; i < m_vInst.size(); i++)
		{
			DEL(m_vInst[i].m_pInst);
		}

		return 0;
	}

	bool Startup::createAllInst(Kiss *pKiss)
	{
		NULL_F(pKiss);

		OK_INST o;
		int i = 0;
		while (1)
		{
			Kiss *pK = pKiss->root()->child(i++);
			if (pK->empty())
				break;

			IF_CONT(pK->m_class == "Startup");

			o.m_pInst = m_module.createInstance(pK);
			if (!o.m_pInst)
			{
				LOG_E("Failed to create instance: " + pK->m_name);
				return false;
			}

			o.m_pKiss = pK;
			m_vInst.push_back(o);
		}

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
