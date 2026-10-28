#include <csignal>
#include "../../src/Instance/InstanceMgr.h"
#include "_AProver_LiDAR.h"

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
		printf("Usage: ./AProver_LiDAR [JSON file]\n");
		return 0;
	}

	string argStr(argv[1]);
	if (argStr == "-h" || argStr == "--help")
	{
		printf("Usage: ./AProver_LiDAR [JSON file]\n");
		return 0;
	}

	printf("Using JSON file: %s\n", argStr.c_str());

	InstanceMgr *pMgr = new InstanceMgr();
	// add user app modules
	_AProver_LiDAR *pAPL = new _AProver_LiDAR();

	if (pMgr == nullptr)
		goto exit;

	if (pAPL == nullptr)
		goto exit;


	signal(SIGINT, signalHandler);
#ifdef USE_GLOG
	FLAGS_logtostderr = 1;
	google::InitGoogleLogging(argv[0]);
#endif


	if (!pMgr->loadJsonFiles(argStr))
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

	if (!pMgr->addExternalModule(pAPL, "apRover"))
		goto exit;

	if (!pMgr->initAll())
		goto exit;

	if (!pMgr->linkAll())
		goto exit;

	if (!pMgr->startAll())
		goto exit;

	while (!g_stopRequested && !pMgr->bComplete())
	{
		sleep(1);
	}

exit:
	// Queue neutral drive and cutter-off while the MAVLink/IO workers are still alive.
	if (pAPL)
		pAPL->stop();
	if (pMgr)
	{
		pMgr->stopAll();
		delete pMgr;
	}

	if (g_stopRequested)
		printf("\nSIGINT\n");

	return 0;
}
