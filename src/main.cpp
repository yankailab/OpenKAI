#include "main.h"

int main(int argc, char* argv[])
{
	//Init Logger
	google::InitGoogleLogging(argv[0]);
	printEnvironment();

	//Load config
	LOG(INFO)<<"Using kiss file: "<<argv[1];
	printf(argv[1]);
	printf("\n");

	string kissFile(argv[1]);
	CHECK_F(!g_file.open(&kissFile));
	string* pKiss = g_file.readAll();
	if(pKiss==NULL)
	{
		LOG(FATAL)<<"Kiss file not found";
		return 1;
	}

	g_pKiss = new Kiss();
	F_FATAL_F(g_pKiss->parse(pKiss));
	g_file.close();

	//Start Application
	g_pGen = new General();
	g_pGen->start(g_pKiss);

	return 0;
}


void printEnvironment(void)
{
	printf("Optimized code: %d\n", useOptimized());
	printf("CUDA devices: %d\n", cuda::getCudaEnabledDeviceCount());
	printf("Current CUDA device: %d\n", cuda::getDevice());

	if (ocl::haveOpenCL())
	{
		printf("OpenCL is found\n");
		ocl::setUseOpenCL(true);
		if (ocl::useOpenCL())
		{
			printf("Using OpenCL\n");
		}
	}
	else
	{
		printf("OpenCL not found\n");
	}
}


