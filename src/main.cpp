#include "main.h"

int main(int argc, char* argv[])
{
	//Init Logger
	google::InitGoogleLogging(argv[0]);
	printEnvironment();

	//Load config
	LOG(INFO)<<"Using config file: "<<argv[1];
	printf(argv[1]);
	printf("\n");
	CHECK_FATAL(g_file.open(argv[1]));
	string config = g_file.getContent();

	g_pConfig = new Config();
	CHECK_FATAL(g_pConfig->parse(config));

	string appName;
	CHECK_FATAL(g_pConfig->o("APP")->v("appName", &appName));

	//Start Application
	g_pGen = new General();
	g_pGen->start(g_pConfig);

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


