#include "Module/ModuleMgr.h"

using namespace kai;

static ModuleMgr *g_pMmgr = nullptr;

void signalHandler(int signal)
{
	if (signal == SIGINT)
	{
		printf("\nSIGINT\n");
		if (g_pMmgr)
		{
			g_pMmgr->stopAll();
		}

		exit(0);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: ./OpenKAI [JSON file]\n");
		return 0;
	}

	string argStr(argv[1]);
	if (argStr == "-h" || argStr == "--help")
	{
		printf("Usage: ./OpenKAI [JSON file]\n");
		return 0;
	}

	printf("Using JSON file: %s\n", argStr.c_str());

	ModuleMgr *pMmgr = new ModuleMgr();
	if (pMmgr == nullptr)
	{
		goto exit;
	}

	g_pMmgr = pMmgr;
	signal(SIGINT, signalHandler);

#ifdef USE_GLOG
	FLAGS_logtostderr = 1;
	google::InitGoogleLogging(argv[0]);
#endif

	// if (!m_bStdErr)
	// 	freopen("/dev/null", "w", stderr);

	if (!pMmgr->parseJsonFile(argStr))
	{
		printf("JSON file parse failed: %s\n", argStr.c_str());
		goto exit;
	}

	if (!pMmgr->createAll())
		goto exit;

	if (!pMmgr->initAll())
		goto exit;

	if (!pMmgr->linkAll())
		goto exit;

	if (!pMmgr->startAll())
		goto exit;

	pMmgr->waitForComplete();

exit:
	delete pMmgr;
	return 0;
}
