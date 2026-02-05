#include "Module/ModuleMgr.h"

using namespace kai;

static ModuleMgr *g_pMgr = nullptr;

void signalHandler(int signal)
{
	if (signal == SIGINT)
	{
		printf("\nSIGINT\n");
		if (g_pMgr)
		{
			g_pMgr->stopAll();
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

	ModuleMgr *pMgr = new ModuleMgr();
	if (pMgr == nullptr)
	{
		goto exit;
	}

	g_pMgr = pMgr;
	signal(SIGINT, signalHandler);

#ifdef USE_GLOG
	FLAGS_logtostderr = 1;
	google::InitGoogleLogging(argv[0]);
#endif

	if (!pMgr->parseJsonFile(argStr))
	{
		printf("JSON file parse failed: %s\n", argStr.c_str());
		goto exit;
	}

	if (!pMgr->bStdErr())
	{
		freopen("/dev/null", "w", stderr);
	}

	if (!pMgr->createAll())
		goto exit;

	if (!pMgr->initAll())
		goto exit;

	if (!pMgr->linkAll())
		goto exit;

	if (!pMgr->startAll())
		goto exit;

	pMgr->waitForComplete();

exit:
	delete pMgr;
	return 0;
}
