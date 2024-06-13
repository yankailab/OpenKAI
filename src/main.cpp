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

	if(!pOK->init())
		goto exit;

	if(!pOK->addKiss(argStr))
		goto exit;

	if(!pOK->createAllModules())
		goto exit;

	if(!pOK->initAllModules())
		goto exit;

	if(!pOK->linkAllModules())
		goto exit;

	if(!pOK->startAllModules())
		goto exit;


	pOK->waitForComplete();

exit:
	delete pOK;
	return 0;
}
