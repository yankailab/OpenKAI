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
		m_pKiss = nullptr;

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

	int OpenKAI::addKiss(const string &fName)
	{
		CHECK_(check());
		IF__(fName.empty(), OK_ERR_INVALID_VALUE);

		string s;
		IF__(!readFile(fName, &s), OK_ERR_NOT_FOUND);

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

		return OK_OK;
	}

	int OpenKAI::addModule(void *pModule, const string &mName)
	{
		CHECK_(check());
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

		return OK_OK;
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
		CHECK_(check());

		Kiss *pK = ((Kiss *)m_pKiss)->find(mName);
		IF__(pK->empty(), OK_ERR_NOT_FOUND);

		pK->setModule(NULL);

		return OK_OK;
	}

	int OpenKAI::createAllModules(void)
	{
		CHECK_(check() != OK_OK);

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

		return OK_OK;
	}

	int OpenKAI::initAllModules(void)
	{
		CHECK_(check());

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

		return OK_OK;
	}

	int OpenKAI::linkAllModules(void)
	{
		CHECK_(check());

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

		return OK_OK;
	}

	int OpenKAI::startAllModules(void)
	{
		CHECK_(check());

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

		return OK_OK;
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
