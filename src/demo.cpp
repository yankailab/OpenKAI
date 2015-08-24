
#include "demo.h"

int main(int argc, char* argv[])
{
	// Initialize Google's logging library.
	google::InitGoogleLogging(argv[0]);

	LOG(INFO)<<"Using config file: "<<argv[1];
	CHECK_FATAL(g_file.open(argv[1]));

	string config = g_file.getContent();
	CHECK_FATAL(g_Json.parse(config.c_str()));

	g_config.setJSON(&g_Json);

	CHECK_ERROR(g_Json.getVal("serialPort", &g_serialPort));

	//Connect to Mavlink
	g_pVehicle = new VehicleInterface();

	if (g_pVehicle->open((char*)g_serialPort.c_str()) != true)
	{
		LOG(ERROR)<< "Cannot open serial port, Working in CV mode only";
	}
	else
	{
		LOG(ERROR) << "Serial port connected";
	}


	printEnvironment();


	//Init CamStream
	g_pCamFront = new CamStream();
	CHECK_FATAL(g_config.setCamStream(g_pCamFront));
	g_pCamFront->init();
	g_pCamFront->openWindow();


	//Init Autopilot
	g_pAP = new AutoPilot();
	CHECK_FATAL(g_config.setAutoPilot(g_pAP));
	g_pAP->init();


	g_pCamFront->start();



	g_bRun = true;
	g_bTracking = false;
	setMouseCallback(APP_NAME, onMouse, NULL);


	//Init Object Detector
//	g_objDet.init(&g_Json);

	while (g_bRun)
	{
		//Handle key input
		g_key = waitKey(1);
		handleKey(g_key);
	}

	g_pCamFront->stop();
	g_pCamFront->complete();


	g_pVehicle->close();
	delete g_pVehicle;
	delete g_pAP;
	delete g_pCamFront;

	return 0;

}


