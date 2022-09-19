#include "main.h"
#include "Script/Kiss.h"
#include "Utility/utilFile.h"

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

	g_pKiss = new Kiss();
	if(!parseKiss(argStr, g_pKiss))
	{
		printf("Kiss file not found or parsing failed\n");
		delete g_pKiss;
		return 1;
	}
    
	g_pStart = new Startup();
	g_pStart->start(g_pKiss);

	delete g_pKiss;
	return 0;
}
