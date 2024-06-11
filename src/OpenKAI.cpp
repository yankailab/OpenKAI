#include "OpenKAI.h"

static OpenKAI *g_pStartup;

void signalHandler(int signal)
{
	if (signal != SIGINT)
		return;
	printf("\nSIGINT: OpenKAI Shutdown\n");

	g_pStartup->stopAllModules();
}

namespace kai
{

	OpenKAI::OpenKAI()
	{
		m_pKiss = NULL;
		m_appName = "";
		m_bRun = true;
		m_bStdErr = true;
		m_bLog = false;
		m_rc = "";
		m_vInst.clear();
	}

	OpenKAI::~OpenKAI()
	{
	}

	int OpenKAI::check(void)
	{
		NULL__(m_pKiss, -1);

		return 0;
	}

	bool OpenKAI::init(void)
	{
		g_pStartup = this;
		signal(SIGINT, signalHandler);

		DEL(m_pKiss);
		m_pKiss = new Kiss();

		return true;
	}

	bool OpenKAI::addModulesFromKiss(const string &fName)
	{
		IF_F(check() < 0);
		IF_F(fName.empty());

		if (!parseKiss(fName, m_pKiss))
		{
			LOG(ERROR) << "Kiss file not found or parsing failed";
			return false;
		}

		Kiss *pApp = m_pKiss->root()->child("APP");
		pApp->v("appName", &m_appName);
		pApp->v("bStdErr", &m_bStdErr);
		pApp->v("bLog", &m_bLog);
		pApp->v("rc", &m_rc);

		// parse & attach included kiss files
		vector<string> vInclude;
		pApp->a("vInclude", &vInclude);
		for (string s : vInclude)
		{
			if (!parseKiss(s, m_pKiss))
			{
				LOG(INFO) << "Kiss parse failed: " << s;
			}
		}

		// logging
		if (!m_bStdErr)
		{
			freopen("/dev/null", "w", stderr);
		}

#ifdef USE_GLOG
		FLAGS_logtostderr = 1;
		google::InitGoogleLogging(m_appName.c_str());
#endif

		return true;
	}

	bool OpenKAI::createAllModules(void)
	{
		IF_F(check() < 0);

		OK_MODULE m;
		int i = 0;
		while (1)
		{
			Kiss *pK = m_pKiss->root()->child(i++);
			if (pK->empty())
				break;

			IF_CONT(pK->m_class == "OpenKAI");

			m.m_pInst = m_module.createInstance(pK);
			if (!m.m_pInst)
			{
				LOG_E("Failed to create instance: " + pK->m_name);
				return false;
			}

			m.m_pKiss = pK;
			m_vInst.push_back(m);
		}

		return true;
	}

	bool OpenKAI::initAllModules(void)
	{
		IF_F(check() < 0);

		for (auto inst : m_vInst)
		{
			IF_CONT(inst.m_pInst->init(inst.m_pKiss));
			LOG_E(inst.m_pKiss->m_name + ".init()");
			return false;
		}

		return true;
	}

	bool OpenKAI::linkAllModules(void)
	{
		IF_F(check() < 0);

		for (auto inst : m_vInst)
		{
			IF_CONT(inst.m_pInst->link());
			LOG_E(inst.m_pKiss->m_name + ".link()");
			return false;
		}

		return true;
	}

	bool OpenKAI::startAllModules(void)
	{
		IF_F(check() < 0);

		// run cmd
		if (!m_rc.empty())
		{
			system(m_rc.c_str());
		}

		for (auto inst : m_vInst)
		{
			IF_CONT(inst.m_pInst->start());
			LOG_E(inst.m_pKiss->m_name + ".start()");
			return false;
		}

		return true;
	}

	void OpenKAI::runAllModules(void)
	{
	}

	void OpenKAI::pauseAllModules(void)
	{
	}

	void OpenKAI::stopAllModules(void)
	{
		m_bRun = false;
	}

	void OpenKAI::waitForComplete(void)
	{
		// TODO: temporal impl
		while (!bComplete())
		{
			sleep(1);
		}
	}

	bool OpenKAI::bComplete(void)
	{
		// TODO: temporal impl
		return false;
	}

	void OpenKAI::exit(void)
	{
		for (auto inst : m_vInst)
		{
			DEL(inst.m_pInst);
		}

		m_vInst.clear();
	}

	void OpenKAI::setName(const string &n)
	{
		m_appName = n;
	}

	string OpenKAI::getName(void)
	{
		return m_appName;
	}

	void OpenKAI::printEnvironment(void)
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
