#include "OpenKAI.h"
#include "Module/Kiss.h"
#include "Utility/utilFile.h"

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

	// OpenKAI *pOK = new OpenKAI();

	// if (pOK->init() != OK_OK)
	// 	goto exit;

	// if (pOK->addKiss(argStr) != OK_OK)
	// {
	// 	printf("Kiss file read failed: %s\n", argStr.c_str());
	// 	goto exit;
	// }

	// if (pOK->createAllModules() != OK_OK)
	// 	goto exit;

	// if (pOK->initAllModules() != OK_OK)
	// 	goto exit;

	// if (pOK->linkAllModules() != OK_OK)
	// 	goto exit;

	// if (pOK->startAllModules() != OK_OK)
	// 	goto exit;

	// pOK->waitForComplete();

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
