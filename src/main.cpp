#include "OpenKAI.h"
#include "Module/Kiss.h"
#include "Utility/utilFile.h"

using namespace kai;

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Usage: ./OpenKAI [kiss file]\n");
		return 0;
	}

	string argStr(argv[1]);
	if(argStr == "-h" || argStr == "--help")
	{
		printf("Usage: ./OpenKAI [kiss file]\n");
		return 0;
	}

	printf("Kiss file: %s\n", argStr.c_str());
    
	OpenKAI* pOK = new OpenKAI();

	if(pOK->init() != OK_OK)
		goto exit;

	if(pOK->addKiss(argStr) != OK_OK)
		goto exit;

	if(pOK->createAllModules() != OK_OK)
		goto exit;

	if(pOK->initAllModules() != OK_OK)
		goto exit;

	if(pOK->linkAllModules() != OK_OK)
		goto exit;

	if(pOK->startAllModules() != OK_OK)
		goto exit;


	pOK->waitForComplete();

exit:
	delete pOK;
	return 0;
}
