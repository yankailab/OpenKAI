#include "OpenKAI.h"
#include "Module/Module.h"
#include "Utility/utilFile.h"

using namespace kai;

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
		m_vModules.clear();
	}

	OpenKAI::~OpenKAI()
	{
	}

	int OpenKAI::check(void)
	{
		NULL__(m_pKiss, -1);

		return 0;
	}

	bool OpenKAI::init(const string& appName)
	{
		g_pStartup = this;
		signal(SIGINT, signalHandler);

		DEL(m_pKiss);
		m_pKiss = new Kiss();

		m_appName = appName;

#ifdef USE_GLOG
		FLAGS_logtostderr = 1;
		google::InitGoogleLogging(m_appName.c_str());
#endif

		return true;
	}

	bool OpenKAI::addKiss(const string &fName)
	{
		IF_F(check() < 0);
		IF_F(fName.empty());

		string s;
		IF_F(!readFile(fName, &s));

		Kiss *pKiss = (Kiss *)m_pKiss;
		IF_Fl(!pKiss->parse(s), "Kiss parse failed: " + s);

		Kiss *pApp = pKiss->root()->child("APP");
		pApp->v("appName", &m_appName);
		pApp->v("bStdErr", &m_bStdErr);
		pApp->v("bLog", &m_bLog);
		pApp->v("rc", &m_rc);

		// parse & attach included kiss files
		vector<string> vInclude;
		pApp->a("vInclude", &vInclude);
		for (string f : vInclude)
		{
			IF_Fl(!readFile(f, &s), "Included Kiss not found: " + f);
			IF_Fl(!pKiss->parse(s), "Included Kiss parse failed: " + f);
		}

		if (!m_bStdErr)
			freopen("/dev/null", "w", stderr);

		return true;
	}

	bool OpenKAI::createAllModules(void)
	{
		IF_F(check() < 0);

		Kiss *pKiss = (Kiss *)m_pKiss;
		Module mod;
		OK_MODULE m;
		int i = 0;
		while (1)
		{
			Kiss *pK = pKiss->root()->child(i++);
			if (pK->empty())
				break;

			IF_CONT(pK->m_class == "OpenKAI");
			IF_CONT(pK->m_pInst);

			m.m_pBase = mod.createInstance(pK);
			if (!m.m_pBase)
			{
				LOG_I("Failed to create instance: " + pK->m_name);
				continue;
			}

			m.m_pKiss = pK;
			m_vModules.push_back(m);
		}

		return true;
	}

	int OpenKAI::addModule(void *pModule, const string &mName)
	{
		IF__(check() < 0, -1);
		NULL__(pModule, -1);

		Kiss *pKiss = (Kiss *)m_pKiss;
		Kiss *pKm = pKiss->find(mName);
		if(pKm->empty())
		{
			LOG_I("Module not found in Kiss: " + mName);
			return -1;
		}

		if(pKm->m_pInst)
		{
			LOG_I("Module already existed in Kiss: " + mName);
			return -1;
		}

		pKm->m_pInst = (BASE*)pModule;

		OK_MODULE m;
		m.m_pBase = pModule;
		m.m_pKiss = pKm;
		m_vModules.push_back(m);
	}

	bool OpenKAI::initAllModules(void)
	{
		IF_F(check() < 0);

		for (auto m : m_vModules)
		{
			Kiss *pKiss = (Kiss *)m.m_pKiss;
			IF_CONT(((BASE *)m.m_pBase)->init(pKiss));

			LOG_E(pKiss->m_name + ".init()");
			return false;
		}

		return true;
	}

	bool OpenKAI::linkAllModules(void)
	{
		IF_F(check() < 0);

		for (auto m : m_vModules)
		{
			IF_CONT(((BASE *)m.m_pBase)->link());
			LOG_E(((Kiss *)m.m_pKiss)->m_name + ".link()");
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

		for (auto m : m_vModules)
		{
			IF_CONT(((BASE *)m.m_pBase)->start());
			LOG_E(((Kiss *)m.m_pKiss)->m_name + ".start()");
			return false;
		}

		return true;
	}

	void OpenKAI::resumeAllModules(void)
	{
		for (auto m : m_vModules)
		{
			((BASE *)m.m_pBase)->resume();
		}
	}

	void OpenKAI::pauseAllModules(void)
	{
		for (auto m : m_vModules)
		{
			((BASE *)m.m_pBase)->pause();
		}
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
		for (auto m : m_vModules)
		{
			DEL(m.m_pBase);
		}

		m_vModules.clear();
	}

	int OpenKAI::findModule(const string &mName)
	{
		for (int i = 0; i < m_vModules.size(); i++)
		{
			IF_CONT(((BASE *)m_vModules[i].m_pBase)->getName() != mName);

			return i;
		}

		return -1;
	}

	int OpenKAI::getModuleHandle(const string &mName)
	{
		return findModule(mName);
	}

	void *OpenKAI::getModule(const string &mName)
	{
		int i = findModule(mName);
		IF_N(i < 0);

		return m_vModules[i].m_pBase;
	}

	bool OpenKAI::startModule(int h)
	{
		IF_F(h <= 0);
		IF_F(h > m_vModules.size());

		BASE *pBase = (BASE *)m_vModules[h].m_pBase;
		pBase->start();

		// TODO: verify already started thread in each ModuleBase instances
	}

	bool OpenKAI::pauseModule(int h)
	{
		IF_F(h <= 0);
		IF_F(h > m_vModules.size());

		BASE *pBase = (BASE *)m_vModules[h].m_pBase;
		pBase->pause();
	}

	bool OpenKAI::resumeModule(int h)
	{
		IF_F(h <= 0);
		IF_F(h > m_vModules.size());

		BASE *pBase = (BASE *)m_vModules[h].m_pBase;
		pBase->resume();
	}

	bool OpenKAI::stopModule(int h)
	{
		IF_F(h <= 0);
		IF_F(h > m_vModules.size());

		BASE *pBase = (BASE *)m_vModules[h].m_pBase;
		pBase->stop();
	}

	bool OpenKAI::deleteModule(int h)
	{
		IF_F(h <= 0);
		IF_F(h > m_vModules.size());

		BASE *pBase = (BASE *)m_vModules[h].m_pBase;
		pBase->stop();

		// TODO:
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
