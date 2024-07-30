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
		m_bStdErr = true;
		m_rc = "";
	}

	OpenKAI::~OpenKAI()
	{
	}

	int OpenKAI::check(void)
	{
		NULL__(m_pKiss, OK_ERR_NULLPTR);

		return OK_OK;
	}

	int OpenKAI::init(const string &appName)
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

		return OK_OK;
	}

	void OpenKAI::clean(void)
	{
		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(!pK->getModule());
			delete pK->getModule();
		}

		DEL(m_pKiss);
	}

	bool OpenKAI::addKiss(const string &fName)
	{
		IF_F(check() != OK_OK);;
		IF_F(fName.empty());

		string s;
		IF_F(!readFile(fName, &s));

		Kiss *pKiss = (Kiss *)m_pKiss;
		IF_Fl(!pKiss->parse(s), "Kiss parse failed: " + s);

		Kiss *pApp = pKiss->root()->child("APP");
		pApp->v("appName", &m_appName);
		pApp->v("bStdErr", &m_bStdErr);
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

	bool OpenKAI::addModule(void *pModule, const string &mName)
	{
		IF_F(check() != OK_OK);;
		NULL_F(pModule);

		Kiss *pKiss = (Kiss *)m_pKiss;
		Kiss *pKm = pKiss->find(mName);

		NULL_Fl(pKm, "Module not found in Kiss: " + mName);
		IF_Fl(pKm->empty(), "Module not found in Kiss: " + mName);
		IF_Fl(pKm->getModule(), "Module already existed in Kiss: " + mName);

		pKm->setModule((BASE *)pModule);

		return true;
	}

	void* OpenKAI::findModule(const string &mName)
	{
		IF_N(check() < 0);

		return ((Kiss*)m_pKiss)->findModule(mName);
	}

	void* OpenKAI::findModuleKiss(const string &mName)
	{
		IF_N(check() < 0);

		return ((Kiss*)m_pKiss)->find(mName);
	}

	bool OpenKAI::deleteModule(const string& mName)
	{
		IF_F(check() != OK_OK);;

		Kiss* pK = ((Kiss*)m_pKiss)->find(mName);
		IF_F(pK->empty());

		pK->setModule(NULL);

		return true;
	}


	bool OpenKAI::createAllModules(void)
	{
		IF_F(check() != OK_OK);;

		Kiss *pKiss = (Kiss *)m_pKiss;
		Module mod;
		int i = 0;
		while (1)
		{
			Kiss *pK = pKiss->root()->child(i++);
			if (pK->empty())
				break;

			IF_CONT(pK->getClass() == "OpenKAI");
			IF_CONT(pK->getModule());

			pK->setModule(mod.createInstance(pK));
			if (!pK->getModule())
			{
				LOG_I("Failed to create instance: " + pK->getName());
				continue;
			}
		}

		return true;
	}

	int OpenKAI::initAllModules(void)
	{
		IF_F(check() != OK_OK);;

		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		int n = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(!pK->getModule());

			if (((BASE *)pK->getModule())->init(pK))
			{
				n++;
			}
			else
			{
				LOG_E(pK->getName() + ".init()");
			}
		}

		return n;
	}

	int OpenKAI::linkAllModules(void)
	{
		IF_F(check() != OK_OK);;

		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		int n = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(!pK->getModule());

			if (((BASE *)pK->getModule())->link())
			{
				n++;
			}
			else
			{
				LOG_E(pK->getName() + ".link()");
			}
		}

		return n;
	}

	int OpenKAI::startAllModules(void)
	{
		IF_F(check() != OK_OK);;

		// run cmd
		if (!m_rc.empty())
		{
			system(m_rc.c_str());
		}

		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		int n = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(!pK->getModule());

			if (((BASE *)pK->getModule())->start())
			{
				n++;
			}
			else
			{
				LOG_E(pK->getName() + ".start()");
			}
		}

		return n;
	}

	void OpenKAI::resumeAllModules(void)
	{
		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(!pK->getModule());
			((BASE *)pK->getModule())->resume();
		}
	}

	void OpenKAI::pauseAllModules(void)
	{
		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(!pK->getModule());
			((BASE *)pK->getModule())->pause();
		}
	}

	void OpenKAI::stopAllModules(void)
	{
		//TODO

		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(!pK->getModule());
			((BASE *)pK->getModule())->stop();
		}
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

	void OpenKAI::setName(const string &n)
	{
		m_appName = n;
	}

	string OpenKAI::getName(void)
	{
		return m_appName;
	}

	void OpenKAI::logEnvironment(void)
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
