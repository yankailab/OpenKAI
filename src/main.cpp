#include "main.h"

int main(int argc, char* argv[])
{
	//Check arg
	if(argc < 2)
	{
		printf("Usage: ./OpenKAI [kiss file]");
		return 0;
	}

	string argStr(argv[1]);
	if(argStr == "-h" || argStr == "--help")
	{
		printf("Usage: ./OpenKAI [kiss file]");
		return 0;
	}

	//Init Logger
	FLAGS_logtostderr = 1;
	google::InitGoogleLogging("OpenKAI");
	printEnvironment();

	LOG(INFO) << "Kiss file: " << argStr;
	if(!g_file.open(&argStr))
	{
		LOG(ERROR) << "Kiss file not found";
		return 1;
	}

	string* pKiss = g_file.readAll();
	if(pKiss==NULL)
	{
		LOG(ERROR) << "Cannot open Kiss file";
		return 1;
	}

	if(pKiss->empty())
	{
		LOG(ERROR) << "Cannot open Kiss file";
		return 1;
	}

	g_pKiss = new Kiss();
	if(!g_pKiss->parse(pKiss))
	{
		LOG(ERROR) << "Kiss file parsing failed";
		return 1;
	}
	g_file.close();

	//Start Application
	g_pStart = new Startup();
	g_pStart->start(g_pKiss);

	return 0;
}

void printEnvironment(void)
{
	LOG(INFO)<<"OpenCV optimized code:"<<useOptimized();
	LOG(INFO)<<"CUDA devices:"<<cuda::getCudaEnabledDeviceCount();
	LOG(INFO)<<"Using CUDA device:"<<cuda::getDevice();

	if (ocl::haveOpenCL())
	{
		LOG(INFO)<<"OpenCL is found";
		ocl::setUseOpenCL(true);
		if (ocl::useOpenCL())
		{
			LOG(INFO)<<"Using OpenCL";
		}
	}
	else
	{
		LOG(INFO)<<"OpenCL not found";
	}
}


