#include "demo.h"

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
	CHECK_FATAL(g_Json.parse(config.c_str()));

	//Start Application
//	g_pAppExtCamControl = new ExtCamControl();
//	g_pAppExtCamControl->start(&g_Json);

	g_pAppSegNetDemo = new SegNetDemo();
	g_pAppSegNetDemo->start(&g_Json);

//	g_pAppDroneHunter = new DroneHunter();
//	g_pAppDroneHunter->start(&g_Json);

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


