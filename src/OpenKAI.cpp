#include "OpenKAI.h"
#include "Module/Module.h"
#include "Utility/utilFile.h"

using namespace kai;

static OpenKAI *g_pStartup;

void signalHandler(int signal)
{
	if (signal == SIGINT)
	{
		printf("\nSIGINT\n");
		g_pStartup->stopAllModules();
		exit(0);
	}
}

namespace kai
{

	OpenKAI::OpenKAI()
	{
		m_pKiss = nullptr;

		m_appName = "";
		m_bStdErr = true;
		m_rc = "";
	}

	OpenKAI::~OpenKAI()
	{
	}

	bool OpenKAI::check(void)
	{
		NULL__(m_pKiss);

		return true;
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

		return true;
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

	int OpenKAI::addKiss(const string &fName)
	{
		IF_F(!check());
		IF__(fName.empty(), OK_ERR_INVALID_VALUE);

		string s;
		IF__(!readFile(fName, &s));

		Kiss *pKiss = (Kiss *)m_pKiss;
		if (!pKiss->parse(s))
		{
			LOG_E("Kiss parse failed: " + s);
			return OK_ERR_INVALID_VALUE;
		}

		Kiss *pApp = pKiss->root()->child("APP");
		pApp->v("appName", &m_appName);
		pApp->v("bStdErr", &m_bStdErr);
		pApp->v("rc", &m_rc);

		// parse & attach included kiss files
		vector<string> vInclude;
		pApp->a("vInclude", &vInclude);
		for (string f : vInclude)
		{
			if (!readFile(f, &s))
			{
				LOG_E("Included Kiss not found: " + f);
				return OK_ERR_NOT_FOUND;
			}

			if (!pKiss->parse(s))
			{
				LOG_E("Included Kiss parse failed: " + f);
				return OK_ERR_INVALID_VALUE;
			}
		}

		if (!m_bStdErr)
			freopen("/dev/null", "w", stderr);

		return true;
	}

	int OpenKAI::addModule(void *pModule, const string &mName)
	{
		IF_F(!check());
		NULL__(pModule, OK_ERR_INVALID_VALUE);

		Kiss *pKiss = (Kiss *)m_pKiss;
		Kiss *pKm = pKiss->find(mName);

		if (pKm == nullptr)
		{
			LOG_E("Module not found in Kiss: " + mName);
			return OK_ERR_NOT_FOUND;
		}

		if (pKm->empty())
		{
			LOG_E("Module not found in Kiss: " + mName);
			return OK_ERR_NOT_FOUND;
		}

		if (pKm->getModule())
		{
			LOG_E("Module already existed in Kiss: " + mName);
			return OK_ERR_DUPLICATE;
		}

		pKm->setModule((BASE *)pModule);

		return true;
	}

	void *OpenKAI::findModule(const string &mName)
	{
		IF__(check() != OK_OK, nullptr);

		return ((Kiss *)m_pKiss)->findModule(mName);
	}

	void *OpenKAI::findModuleKiss(const string &mName)
	{
		IF__(check() != OK_OK, nullptr);

		return ((Kiss *)m_pKiss)->find(mName);
	}

	int OpenKAI::deleteModule(const string &mName)
	{
		IF_F(!check());

		Kiss *pK = ((Kiss *)m_pKiss)->find(mName);
		IF__(pK->empty());

		pK->setModule(NULL);

		return true;
	}

	int OpenKAI::createAllModules(void)
	{
		IF_F(!check() != OK_OK);

		Kiss *pKiss = (Kiss *)m_pKiss;
		Module mod;
		int i = 0;
		while (1)
		{
			Kiss *pK = pKiss->root()->child(i++);
			if (pK->empty())
				break;

			IF_CONT(pK->getClass() == "OpenKAI");
			IF_CONT(pK->getModule() != nullptr);

			pK->setModule(mod.createInstance(pK));
			if (pK->getModule() == nullptr)
			{
				LOG_I("Failed to create instance: " + pK->getName());
				continue;
			}
		}

		return true;
	}

	int OpenKAI::initAllModules(void)
	{
		IF_F(!check());

		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(pK->getModule() == nullptr);

			if (((BASE *)pK->getModule())->init(pK) != OK_OK)
			{
				LOG_E(pK->getName() + ".init()");
			}
		}

		return true;
	}

	int OpenKAI::linkAllModules(void)
	{
		IF_F(!check());

		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(pK->getModule() == nullptr);

			if (((BASE *)pK->getModule())->link() != OK_OK)
			{
				LOG_E(pK->getName() + ".link()");
			}
		}

		return true;
	}

	int OpenKAI::startAllModules(void)
	{
		IF_F(!check());

		// run cmd
		if (!m_rc.empty())
		{
			system(m_rc.c_str());
		}

		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(pK->getModule() == nullptr);

			if (((BASE *)pK->getModule())->start() != OK_OK)
			{
				LOG_E(pK->getName() + ".start()");
			}
		}

		return true;
	}

	void OpenKAI::resumeAllModules(void)
	{
		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(pK->getModule() == nullptr);
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
			IF_CONT(pK->getModule() == nullptr);
			((BASE *)pK->getModule())->pause();
		}
	}

	void OpenKAI::stopAllModules(void)
	{
		// TODO

		Kiss *pKroot = ((Kiss *)m_pKiss)->root();
		Kiss *pK;
		int i = 0;
		while (!(pK = pKroot->child(i++))->empty())
		{
			IF_CONT(pK->getModule() == nullptr);
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
