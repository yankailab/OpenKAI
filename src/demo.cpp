
#include "demo.h"

int main(int argc, char* argv[])
{
	// Initialize Google's logging library.
	google::InitGoogleLogging(argv[0]);

	namedWindow(APP_NAME);

	if (!g_file.open(argv[1]))
	{
		LOG(FATAL)<<"FileIO.open()";
		return 1;
	}
	LOG(INFO)<<"Using config file: "<<argv[1];

	string config = g_file.getContent();
	if (!g_Json.parse(config.c_str()))
	{
		LOG(FATAL)<<"JSON.parse()";
		return 1;
	}

	if (!g_Json.getVal("serialPort", &g_serialPort))
	{
		LOG(FATAL)<<"JSON.getVal():serialPort";
		return 1;
	}

	g_bRun = true;
	g_bTracking = false;

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




	//Open front camera
	CHECK_FATAL(g_Json.getVal("CAM_FRONT_ID", &g_camFront.m_camDeviceID));
	if(!g_camFront.openCamera())
	{
		LOG(FATAL)<< "Cannot open External camera\n";
		return 1;
	}

	CHECK_FATAL(g_Json.getVal("CAM_FRONT_WIDTH", &g_camFront.m_width));
	CHECK_FATAL(g_Json.getVal("CAM_FRONT_HEIGHT", &g_camFront.m_height));
	g_camFront.setSize();

	//Initial target position
	g_targetPosExt.m_x = g_camFront.m_width * 0.5;
	g_targetPosExt.m_y = g_camFront.m_height * 0.5;
	g_targetPosExt.m_z = (Z_NEAR_LIM + Z_FAR_LIM) * 0.2;


	//Init frames
	g_frontRGB.init();

	//Init Marker Detector
	g_markerDet.init();



	//Init Autopilot
	g_pAP = new AutoPilot();
	g_pAP->init(&g_Json);
	g_pAP->setTargetPosCV(g_targetPosExt);

	//Mat for display output
	g_displayMat = Mat(INFOWINDOW_WIDTH, INFOWINDOW_HEIGHT, CV_8UC3, Scalar(0));

	//set the callback function for any mouse event
	setMouseCallback(APP_NAME, onMouse, NULL);


	//Init Object Detector
//	g_ObjDet = new ObjectDetector();
	g_objDet.init(&g_Json);






	while (g_bRun)
	{
		g_frontRGB.switchFrame();
		g_camFront.readFrame(&g_frontRGB);

		g_frontHSV.switchFrame();
		g_frontRGB.getHSV(&g_frontHSV);

		g_markerDet.detect(&g_frontHSV);

		g_pAP->markerLock(&g_markerDet,g_pVehicle);


		g_numObj = g_objDet.detect(g_frontRGB.m_uFrame/*.getMat(ACCESS_READ)*/,&g_pObj);

//		displayInfo();
		imshow(APP_NAME, g_displayMat);

		//Handle key input
		g_key = waitKey(1);
		handleKey(g_key);
	}

	g_camFront.m_camera.release();
	g_pVehicle->close();
	delete g_pVehicle;
	delete g_pAP;

	return 0;

}


