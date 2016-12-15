#include "main.h"

int main(int argc, char* argv[])
{
	//Init Logger
	FLAGS_logtostderr = 1;
	google::InitGoogleLogging("OpenKAI");
	printEnvironment();

	//Load config
	LOG(INFO)<<"Kiss file:"<<argv[1];

	string kissFile(argv[1]);
	CHECK_F(!g_file.open(&kissFile));
	string* pKiss = g_file.readAll();
	if(pKiss==NULL)
	{
		LOG(FATAL)<<"Kiss file not found";
		return 1;
	}

	g_pKiss = new Kiss();
	if(!g_pKiss->parse(pKiss))
	{
		LOG(FATAL)<<"Kiss file parsing failed";
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
	LOG(INFO)<<"Optimized code:"<<useOptimized();
	LOG(INFO)<<"CUDA devices:"<<cuda::getCudaEnabledDeviceCount();
	LOG(INFO)<<"Current CUDA device:"<<cuda::getDevice();

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


