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
	if(!g_file.open(&argStr))
	{
		printf("Kiss file not found\n");
		return 1;
	}

	string* pKiss = g_file.readAll();
	if(!pKiss)
	{
		printf("Cannot open Kiss file\n");
		return 1;
	}

	if(pKiss->empty())
	{
		printf("Kiss file is empty\n");
		return 1;
	}

	g_pKiss = new Kiss();
	if(!g_pKiss->parse(pKiss))
	{
		printf("Kiss file parsing failed\n");
		return 1;
	}
	g_file.close();

	g_pStart = new Startup();
	g_pStart->start(g_pKiss);

	return 0;
}
