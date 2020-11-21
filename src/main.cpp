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

    fstream f;
    f.open(argStr.c_str(), ios::in);
    if(!f.is_open())
	{
		printf("Kiss file not found\n");
		return 1;
	}
	f.seekg(0, ios_base::beg);

	string kiss = "";
	while (f && !f.eof())
	{
        string s;
        getline(f, s);
		kiss += s;
	}

	if( kiss.empty())
	{
		printf("Kiss file is empty\n");
		return 1;
	}

	g_pKiss = new Kiss();
	if(!g_pKiss->parse(&kiss))
	{
		printf("Kiss file parsing failed\n");
		return 1;
	}
    
	g_pStart = new Startup();
	g_pStart->start(g_pKiss);

	return 0;
}
