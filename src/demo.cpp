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
	CHECK_ERROR(g_Json.getVal("serialPortMavlink", &g_serialPort));
	CHECK_ERROR(g_Json.getVal("baudrateMavlink", &g_baudrate));

	g_pMavlink = new MavlinkInterface();
	g_pMavlink->setSerial(g_serialPort, g_baudrate);
	if (g_pMavlink->open())
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
	g_pOD->m_pCamStream = g_pCamFront;

	//Init Fast Detector
	g_pFD = new FastDetector();
	g_pFD->init(&g_Json);
	g_pFD->m_pCamStream = g_pCamFront;

#endif

	//Init Autopilot
	g_pAP = new AutoPilot();
	CHECK_FATAL(g_config.setAutoPilot(g_pAP));
	g_pAP->init();
	g_pAP->setCamStream(g_pCamFront, CAM_FRONT);
//	g_pAP->setVehicleInterface(g_pVehicle);
#ifdef OBJECT_DETECT
	g_pAP->m_pOD = g_pOD;
	g_pAP->m_pFD = g_pFD;
#endif
#ifdef MARKER_COPTER
	g_pMD = g_pAP->m_pCamStream[CAM_FRONT].m_pCam->m_pMarkerDetect;
#endif

	//Start threads
	g_pCamFront->start();
	g_pMavlink->start();
#ifdef MARKER_COPTER
	g_pAP->start();
#endif
#ifdef OBJECT_DETECT
	g_pOD->start();
	g_pFD->start();
#endif

	//UI thread
	int i;
	g_bRun = true;
	setMouseCallback(g_pCamFront->m_camName, onMouse, NULL);

	while (g_bRun)
	{
		Mavlink_Messages mMsg;
		mMsg = g_pMavlink->current_messages;
		g_pCamFront->m_rotate = mMsg.attitude.roll;
		printf("%f\n", g_pCamFront->m_rotate);

		if (!g_pCamFront->m_pMonitor->m_mat.empty())
		{
#ifdef OBJECT_DETECT
			Mat imL, imR, imD;

//			g_pCamFront->m_pFrameL->m_pNext->download(imL);
			g_pCamFront->m_pFrameL->m_tmpMat.download(imL);

			if (!imL.empty())
			{
				OBJECT* pObj;
				for (i = 0; i < g_pOD->m_numObj; i++)
				{
					pObj = &g_pOD->m_pObjects[i];
					if (pObj->m_name[0].empty())
						continue;

					rectangle(imL, pObj->m_boundBox.tl(),
							pObj->m_boundBox.br(), Scalar(0, 255, 0), 2, 5, 0);

					putText(imL, pObj->m_name[0],
							pObj->m_boundBox.tl(), FONT_HERSHEY_SIMPLEX, 0.6,
							Scalar(255, 0, 0), 2);
				}

				FAST_OBJECT* pFastObj;
				for (i = 0; i < g_pFD->m_numHuman; i++)
				{
					pFastObj = &g_pFD->m_pHuman[i];

					rectangle(imL, pFastObj->m_boundBox.tl(),
							pFastObj->m_boundBox.br(), Scalar(0, 0, 255), 2, 5, 0);
				}


				//			g_pCamFront->m_pMonitor->show();
				displayInfo(&imL);


				Mat gimbal = imL(Rect(240, 100, 800, 600));
//				Mat gimbal = imL(Rect(190, 90, 1600, 900));
				imshow("Left", gimbal);

//				imshow("Left", imL);
				if(g_pOD->m_saliencyMap.rows != 0)
				{
					imshow( "Saliency Map", g_pOD->m_saliencyMap );
				}

				if(g_pOD->m_binMap.rows != 0)
				{
					imshow( "Binary Map", g_pOD->m_binMap );
				}
			}



			if (g_pCamFront->m_bStereoCam)
			{
				g_pCamFront->m_pFrameR->m_pNext->download(imR);
				if (!imR.empty())
				{
					imshow("Right", imR);
				}
				imD = Mat(imD.size(), CV_8U);
				g_pCamFront->m_pDepth->m_pNext->download(imD);
				if (!imD.empty())
				{
					imshow("Stereo", imD);
				}
			}

#endif

#ifdef MARKER_COPTER
			displayInfo();
			imshow(g_pCamFront->m_camName, g_displayMat);
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
	g_pMavlink->stop();

#ifdef OBJECT_DETECT
	g_pOD->complete();
#endif
	g_pAP->complete();
	g_pCamFront->complete();
	g_pMavlink->complete();

	g_pMavlink->close();

	delete g_pMavlink;
	delete g_pAP;
	delete g_pCamFront;

#ifdef OBJECT_DETECT
	delete g_pOD;
#endif

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

void handleKey(int key)
{
	switch (key)
	{
	case 'a':
		break;
	case 'z':
		break;
	case 'c':
		break;
	case 'm':
		break;
	case 27:
		g_bRun = false;	//ESC
		break;
	default:
		break;
	}
}

void onMouse(int event, int x, int y, int flags, void* userdata)
{
//	if (x > CAM_WIDTH || y > CAM_HEIGHT)return;

	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		break;
	case EVENT_LBUTTONUP:
		break;
	case EVENT_MOUSEMOVE:
		break;
	case EVENT_RBUTTONDOWN:
		break;
	default:
		break;
	}
}

void onTrackbar(int, void*)
{
}

void createConfigWindow(void)
{
}

void displayInfo(Mat* pDisplayMat)
{
	char strBuf[512];
	std::string strInfo;
	cv::Rect roi;
	cv::Mat* pExtMat;
	int i;
	int startPosH = 25;
	int startPosV = 25;
	int lineHeight = 20;
	Mavlink_Messages mMsg;

	i = 0;

//	g_pCamFront->m_pMonitor->m_mat.copyTo(g_displayMat);

	//External Camera Output
//	pExtMat = &g_frontRGB.m_uFrame;
//	roi = cv::Rect(cv::Point(0, 0), pExtMat->size());
//	pExtMat->copyTo(g_displayMat(roi));

	mMsg = g_pMavlink->current_messages;

	//Vehicle position
	sprintf(strBuf, "Attitude: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			mMsg.attitude.roll,
			mMsg.attitude.pitch,
			mMsg.attitude.yaw);
	cv::putText(*pDisplayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	sprintf(strBuf, "Speed: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			mMsg.attitude.rollspeed,
			mMsg.attitude.pitchspeed,
			mMsg.attitude.yawspeed);
	cv::putText(*pDisplayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

/*	sprintf(strBuf, "Attitude: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			mMsg.attitude.roll,
			mMsg.attitude.pitch,
			mMsg.attitude.yaw);
	cv::putText(*pDisplayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
*/
	i++;

	i = 0;
	startPosH = 600;

/*	sprintf(strBuf, "PITCH_I_PWM: %.2f",
			pControl.m_cvErrInteg * pControl.m_I);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
*/
}















