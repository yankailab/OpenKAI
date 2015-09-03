
#include "demo.h"

//test
int main(int argc, char* argv[])
{
	// Initialize Google's logging library.
	google::InitGoogleLogging(argv[0]);

	printEnvironment();

	LOG(INFO)<<"Using config file: "<<argv[1];
	CHECK_FATAL(g_file.open(argv[1]));

	string config = g_file.getContent();
	CHECK_FATAL(g_Json.parse(config.c_str()));

	g_config.setJSON(&g_Json);

	//Connect to Mavlink
	g_pVehicle = new VehicleInterface();
	CHECK_ERROR(g_Json.getVal("serialPort", &g_serialPort));
	g_pVehicle->setSerialName(g_serialPort);
	if(g_pVehicle->open())
	{
		printf("Serial port opened¥n");
	}

	//Init CamStream
	g_pCamFront = new CamStream();
	CHECK_FATAL(g_config.setCamStream(g_pCamFront));
	g_pCamFront->init();
	g_pCamFront->openWindow();

#ifdef OBJECT_DETECT
	g_pCamFront->m_bGray = true;
#endif
#ifdef MARKER_COPTER
	g_pCamFront->m_bHSV = true;
	g_pCamFront->m_bMarkerDetect = true;
//	g_pCamFront->m_bDenseFlow = true;
#endif

#ifdef OBJECT_DETECT
	//Init Object Detector
	g_pOD = new ObjectDetector();
	g_pOD->init(&g_Json);
#endif

	//Init Autopilot
	g_pAP = new AutoPilot();
	CHECK_FATAL(g_config.setAutoPilot(g_pAP));
	g_pAP->init();
	g_pAP->setCamStream(g_pCamFront,CAM_FRONT);
	g_pAP->setVehicleInterface(g_pVehicle);
#ifdef OBJECT_DETECTOR
	g_pAP->m_pOD = g_pOD;
#endif
	g_pMD = g_pAP->m_pCamStream[CAM_FRONT].m_pCam->m_pMarkerDetect;

	//Start threads
//	g_pVehicle->start();
	g_pCamFront->start();
	g_pAP->start();
#ifdef OBJECT_DETECT
	g_pOD->start();
#endif

	//UI thread
	g_bRun = true;
	g_bTracking = false;
	setMouseCallback(g_pCamFront->m_camName, onMouse, NULL);

	while (g_bRun)
	{

		if(!g_pCamFront->m_pMonitor->m_mat.empty())
		{
#ifdef OBJECT_DETECT
			DETECTOR_STREAM* pDS = &g_pOD->m_pStream[CAM_FRONT];
			NN_OBJECT* pObj;
			for (int i = 0; i < pDS->m_numObj; i++)
			{
				pObj = &pDS->m_pObjects[i];
				if(pObj->m_name[0].empty())continue;
//				if(pObj->m_prob[0]<0.5)continue;

				rectangle(g_pCamFront->m_pMonitor->m_mat,
						pObj->m_boundBox.tl(),
						pObj->m_boundBox.br(), Scalar(0,255,0), 2, 5,0);

				cv::putText(g_pCamFront->m_pMonitor->m_mat,
							pObj->m_name[0],
							pObj->m_boundBox.tl(),
							FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2);
			}

			g_pCamFront->m_pMonitor->show();
#endif

#ifdef MARKER_COPTER
			displayInfo();
			imshow(g_pCamFront->m_camName,g_displayMat);
#endif
		}

		//Handle key input
		g_key = waitKey(30);
		handleKey(g_key);
	}

#ifdef OBJECT_DETECT
	g_pOD->stop();
#endif
	g_pAP->stop();
	g_pCamFront->stop();
	g_pVehicle->stop();

#ifdef OBJECT_DETECT
	g_pOD->complete();
#endif
	g_pAP->complete();
	g_pCamFront->complete();
	g_pVehicle->complete();

	g_pVehicle->close();

	delete g_pVehicle;
	delete g_pAP;
	delete g_pCamFront;

#ifdef OBJECT_DETECT
	delete g_pOD;
#endif

	return 0;

}


