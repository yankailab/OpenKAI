#include "Module/ModuleMgr.h"
#include <csignal>

using namespace kai;

static volatile std::sig_atomic_t g_stopRequested = 0;

void signalHandler(int signal)
{
	if (signal == SIGINT)
	{
		// Defer cleanup to the main thread; it is not signal-safe.
		g_stopRequested = 1;
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

	if (g_stopRequested || !pMgr->createAll())
		goto exit;

	if (g_stopRequested || !pMgr->initAll())
		goto exit;

	if (g_stopRequested || !pMgr->linkAll())
		goto exit;

	if (g_stopRequested || !pMgr->startAll())
		goto exit;

	while (!g_stopRequested && !pMgr->bComplete())
	{
		sleep(1);
	}

exit:
	if (pMgr)
	{
		pMgr->stopAll();
		delete pMgr;
	}

	if (g_stopRequested)
		printf("\nSIGINT\n");
	return 0;
}
