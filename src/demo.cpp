
#include "demo.h"

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

	//Init CamStream
	g_pCamFront = new CamStream();
	CHECK_FATAL(g_config.setCamStream(g_pCamFront));
	g_pCamFront->init();
	g_pCamFront->openWindow();
	g_pCamFront->m_bGray = true;

	//Init Object Detector
	g_pOD = new ObjectDetector();
	g_pOD->init(&g_Json);

	//Init Autopilot
	g_pAP = new AutoPilot();
	CHECK_FATAL(g_config.setAutoPilot(g_pAP));
	g_pAP->init();
	g_pAP->setCamStream(g_pCamFront,CAM_FRONT);
	g_pAP->setVehicleInterface(g_pVehicle);
	g_pAP->m_pOD = g_pOD;

	//Start threads
	g_pVehicle->start();
	g_pCamFront->start();
	g_pAP->start();
	g_pOD->start();

	//UI thread
	g_bRun = true;
	g_bTracking = false;
	setMouseCallback(APP_NAME, onMouse, NULL);

	//Wait until the first frame
//	while(g_pCamFront->m_pMonitor->m_mat.empty());

	while (g_bRun)
	{
		if(!g_pCamFront->m_pMonitor->m_mat.empty())
		{

			DETECTOR_STREAM* pDS = &g_pOD->m_pStream[CAM_FRONT];
			NN_OBJECT* pObj;
			for (int i = 0; i < pDS->m_numObj; i++)
			{
				pObj = &pDS->m_pObjects[i];
				if(pObj->m_name[0].empty())continue;

				rectangle(g_pCamFront->m_pMonitor->m_mat,
						pObj->m_boundBox.tl(),
						pObj->m_boundBox.br(), Scalar(0,255,0), 2, 5,0);

				cv::putText(g_pCamFront->m_pMonitor->m_mat,
							pObj->m_name[0],
							pObj->m_boundBox.tl(),
							FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
			}

			g_pCamFront->m_pMonitor->show();
		}

		//Handle key input
		g_key = waitKey(30);
		handleKey(g_key);
	}

	g_pOD->stop();
	g_pAP->stop();
	g_pCamFront->stop();
	g_pVehicle->stop();

	g_pOD->complete();
	g_pAP->complete();
	g_pCamFront->complete();
	g_pVehicle->complete();

	g_pVehicle->close();

	delete g_pVehicle;
	delete g_pAP;
	delete g_pCamFront;
//	delete g_pOD;

	return 0;

}


