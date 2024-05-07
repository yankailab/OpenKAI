#include "main.h"

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
    
	g_pStartup = new Startup();
	g_pStartup->start(argStr);

	delete g_pStartup;
	return 0;
}
