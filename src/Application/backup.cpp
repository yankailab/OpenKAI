//
//#include "ExtCamControl.h"
//
//bool extCamControl(JSON* pJson)
//{
//	//Init Camera
//	g_pCamFront = new _CamStream();
//	CHECK_FATAL(g_pCamFront->init(pJson, "FRONTL"));
//
//	//Init Marker Detector
//	g_pMD = new _MarkerDetector();
//	CHECK_FATAL(g_pMD->init(pJson, "RED_CIRCLE"));
//	g_pMD->m_pCamStream = g_pCamFront;
//	g_pCamFront->m_bHSV = true;
//
//	//Init BgFg Detector
//	g_pBgFgD = new _BgFgDetector();
//	CHECK_FATAL(g_pBgFgD->init(pJson, ""));
//	g_pBgFgD->m_pCamStream = g_pCamFront;
//
//	//Init Optical Flow
///*	g_pDF = new _DenseFlow();
//	CHECK_FATAL(g_pDF->init(&g_Json, "FRONTL"));
//	g_pDF->m_pGray = g_pCamFront->m_pGrayL;
//	g_pDF->m_pCamStream = g_pCamFront;
//	g_pCamFront->m_bGray = true;
//*/
//
//	//Init Fast Detector
///*	g_pFD = new _CascadeDetector();
//	g_pFD->init("DRONE", &g_Json);
//	g_pFD->m_pGray = g_pCamFront->m_pGrayL;
//	g_pFD->m_pCamStream = g_pCamFront;
//	g_pCamFront->m_bGray = true;
//*/
//
//	//Init SegNet
///*	g_pSegNet = new _SegNet();
//	g_pSegNet->init("DEFAULT",&g_Json);
//	g_pSegNet->m_pFrame = g_pCamFront->m_pFrameL;
//	g_pSegNet->m_pCamStream = g_pCamFront;
//	g_pCamFront->m_bGray = true;
//*/
//
//	//Init Autopilot
///*	g_pAP = new _AutoPilot();
//	CHECK_FATAL(g_pAP->setup(&g_Json, ""));
//	g_pAP->init();
//	g_pAP->setCamStream(g_pCamFront, CAM_FRONT);
//	g_pAP->m_pOD = g_pOD;
//	g_pAP->m_pFD = g_pFD;
////	g_pMD = g_pAP->m_pCamStream[CAM_FRONT].m_pCam->m_pMarkerDetect;
//*/
//
//	//Init Classifier Manager
////	g_pClassMgr = new _ClassifierManager();
////	g_pClassMgr->init(&g_Json);
//
//	//Init Object Detector
///*	g_pOD = new _ObjectDetector();
//	g_pOD->init(&g_Json);
//	g_pOD->m_pClassMgr = g_pClassMgr;
//	//Temporal
//	g_pOD->m_bOneImg = 1;
//*/
//
//	//Connect to Mavlink
///*	g_pMavlink = new _MavlinkInterface();
//	CHECK_FATAL(g_pMavlink->setup(&g_Json, "FC"));
//	CHECK_INFO(g_pMavlink->open());
//*/
//
//	//Main window
//	g_pShow = new CamFrame();
//	g_pMat = new CamFrame();
//	g_pMat2 = new CamFrame();
//
//	//Init UI Monitor
//	g_pUIMonitor = new UIMonitor();
//	g_pUIMonitor->init("OpenKAI demo", &g_Json);
//	g_pUIMonitor->addFullFrame(g_pShow);
//
//	//Start threads
//	g_pCamFront->start();
////	g_pMavlink->start();
////	g_pClassMgr->start();
////	g_pOD->start();
////	g_pDF->start();
//	g_pMD->start();
////	g_pAP->start();
////	g_pFD->start();
////	g_pSegNet->start();
//	g_pBgFgD->start();
//
//	//UI thread
//	g_bRun = true;
//	namedWindow("CopterDetector", CV_WINDOW_NORMAL);
//	setWindowProperty("CopterDetector", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
//	setMouseCallback("CopterDetector", onMouse, NULL);
//
//	while (g_bRun)
//	{
////		Mavlink_Messages mMsg;
////		mMsg = g_pMavlink->current_messages;
////		g_pCamFront->m_pCamL->m_bGimbal = true;
////		g_pCamFront->m_pCamL->setAttitude(mMsg.attitude.roll, 0, mMsg.time_stamps.attitude);
//
////		showScreenSegNet();
////		showScreenCascadeDetector();
////		showScreenDenseFlow();
//		if(!showScreenMarkerDetector())
//		{
//			showScreenBgFgDetector();
//		}
//
//		//Handle key input
//		g_key = waitKey(30);
//		handleKey(g_key);
//	}
//
////	g_pDF->stop();
////	g_pOD->stop();
////	g_pFD->stop();
////	g_pAP->stop();
//	g_pMD->stop();
////	g_pCamFront->stop();
////	g_pMavlink->stop();
////	g_pClassMgr->stop();
////	g_pSegNet->stop();
//	g_pBgFgD->stop();
//
//	g_pMD->complete();
//	g_pBgFgD->complete();
////	g_pDF->complete();
////	g_pClassMgr->complete();
////	g_pOD->complete();
////	g_pFD->complete();
////	g_pAP->complete();
////	g_pCamFront->complete();
////	g_pMavlink->complete();
////	g_pMavlink->close();
//
////	delete g_pClassMgr;
////	delete g_pAP;
////	delete g_pMavlink;
//	delete g_pDF;
//	delete g_pCamFront;
////	delete g_pOD;
////	delete g_pFD;
//
//	return 0;
//
//}
//
//void showScreenSegNet(void)
//{
//	int i;
//	Mat imMat,imMat2,imMat3;
//	CamFrame* pFrame = (*g_pCamFront->m_pFrameProcess);
//
//	if (pFrame->getCurrentFrame()->empty())return;
////	if (g_pShow->isNewerThan(pFrame))return;
//	if (g_pSegNet->m_segment.empty())return;
//
//	pFrame->getCurrentFrame()->download(imMat);
//
//	g_pMat->updateFrame(&g_pSegNet->m_segment);
//	g_pMat2->getResizedOf(g_pMat, imMat.cols,imMat.rows);
//	g_pMat2->getCurrentFrame()->download(imMat2);
//
//	cv::addWeighted(imMat, 1.0, imMat2, 0.5, 0.0, imMat3);
//
//	putText(imMat3, "Camera FPS: "+f2str(g_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat3, "SegNet FPS: "+f2str(g_pSegNet->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//
//	imshow("SegNet",imMat3);
//
////	g_pShow->updateFrame(&imMat3);
////	g_pUIMonitor->show();
//
//}
//
//void showScreenDenseFlow(void)
//{
//	if(!g_pDF->m_showMat.empty())
//	{
//		imshow("DenseFlow",g_pDF->m_showMat);
//	}
//}
//
//bool showScreenMarkerDetector(void)
//{
//	int i;
//	Mat imMat;
//	CamFrame* pFrame = (*g_pCamFront->m_pFrameProcess);
//
//	if (pFrame->getCurrentFrame()->empty())return false;
//	pFrame->getCurrentFrame()->download(imMat);
//
//	putText(imMat, "Camera FPS: "+f2str(g_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat, "Marker Detector FPS: "+f2str(g_pMD->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//
//	fVector3 markerPos;
//	int lockLevel = g_pMD->getObjLockLevel();
//	if (lockLevel < LOCK_LEVEL_POS)
//	{
//		return false;
//	}
//
//	putText(imMat, "MARKER LOCKED", cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	//Update current position with trajectory estimation
//	g_pMD->getObjPosition(&markerPos);
//	circle(imMat, Point(markerPos.m_x,markerPos.m_y), markerPos.m_z, Scalar(0, 255, 0), 2);
//
//	imshow("CopterDetector",imMat);
//	return true;
//}
//
//void showScreenBgFgDetector(void)
//{
//	int i;
//	Mat imMat, imMat2;
//	CamFrame* pFrame = (*g_pCamFront->m_pFrameProcess);
//
//	if(pFrame->getCurrentFrame()->empty())return;
//	if(g_pBgFgD->m_Mat.empty())return;
//	if(g_pBgFgD->m_gFg.empty())return;
//
////	g_pBgFgD->m_gFg.download(imMat2);
////	imshow("Bg",imMat);
////	imshow("Fg",imMat2);
////	imshow("BgFgContour",g_pBgFgD->m_Mat);
//
//	imMat = g_pBgFgD->m_Mat;
//
//	putText(imMat, "Camera FPS: "+f2str(g_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat, "BgFg Detector FPS: "+f2str(g_pBgFgD->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//
////	fVector3 markerPos;
////	int lockLevel = g_pMD->getObjLockLevel();
////	if (lockLevel >= LOCK_LEVEL_POS)
////	{
////		putText(imMat, "BgFg", cv::Point(15,55), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
////		//Update current position with trajectory estimation
////		g_pMD->getObjPosition(&markerPos);
////		circle(imMat, Point(markerPos.m_x,markerPos.m_y), markerPos.m_z, Scalar(0, 255, 0), 2);
////	}
//
//	imshow("CopterDetector",imMat);
//}
//
//void showScreenCascadeDetector(void)
//{
//	int i;
//	Mat imMat;
//	CamFrame* pFrame = (*g_pCamFront->m_pFrameProcess);
//
//	if (pFrame->getCurrentFrame()->empty())return;
//	pFrame->getCurrentFrame()->download(imMat);
//
//	CASCADE_OBJECT* pDrone;
//	int iTarget = 0;
//
//	for (i = 0; i < g_pFD->m_numObj; i++)
//	{
//		pDrone = &g_pFD->m_pObj[i];
//		if(pDrone->m_status != OBJ_ADDED)continue;
//
//		if(iTarget == 0)iTarget = i;
//		rectangle(imMat, pDrone->m_boundBox.tl(), pDrone->m_boundBox.br(), Scalar(0, 0, 255), 2);
//	}
//
//	pDrone = &g_pFD->m_pObj[iTarget];
//	putText(imMat, "LOCK: DJI Phantom", Point(pDrone->m_boundBox.tl().x,pDrone->m_boundBox.tl().y-20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);
//
//	putText(imMat, "Camera FPS: "+f2str(g_pCamFront->getFrameRate()), cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//	putText(imMat, "Cascade FPS: "+f2str(g_pFD->getFrameRate()), cv::Point(15,35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//
//	imshow("CascadeDetector",imMat);
//
////	g_pShow->updateFrame(&imMat3);
////	g_pUIMonitor->show();
//
//
//		//	Mat matShow;
//		//	m_Thresh.download(matShow);
//		//	m_Mat.m_pSmallNew->download(matShow);
//
//			// Storage for blobs
//		//	vector<KeyPoint> keypoints;
//			// Detect blobs
//		//	m_pBlobDetector->detect(matShow, keypoints);
//
//			// Draw detected blobs as red circles.
//			// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
//			// the size of the circle corresponds to the size of blob
//
//		//	Mat imTest;
//		//	drawKeypoints(matShow, keypoints, imTest, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
//
//		//	imshow(MF_WINDOW, matThresh);
//
//		//	m_objLockLevel = LOCK_LEVEL_NONE;
//		//	return;
//
//
///*
//	int i;
//	Mat imMat;
//	vector< vector< Point > > contours;
//	CamFrame* pFrame = (*g_pCamFront->m_pFrameProcess);
//
//	if (pFrame->getCurrentFrame()->empty())return;
//	if (g_pShow->isNewerThan(pFrame))return;
//
//	g_pShow->updateFrame(pFrame);
//	g_pShow->getCurrentFrame()->download(imMat);
//
//	OBJECT* pObj;
//	for (i = 0; i < g_pClassMgr->m_numObj; i++)
//	{
//		pObj = &g_pClassMgr->m_pObjects[i];
//		contours.clear();
//		contours.push_back(pObj->m_vContours);
//
//		//Green
//		if(pObj->m_status == OBJ_COMPLETE)
//		{
//			if (pObj->m_name[0].empty())continue;
//
//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(),
//					Scalar(0, 255, 0), 2, 5, 0);
//
////			drawContours(imMat, contours, -1, Scalar(0, 255, 0),2);
//
//			putText(imMat, pObj->m_name[0], pObj->m_boundBox.tl(),
//					FONT_HERSHEY_SIMPLEX, 0.9, Scalar(255, 0, 0), 2);
//		}
//
//		//Yellow
//		if(pObj->m_status == OBJ_CLASSIFYING)
//		{
////			drawContours(imMat, contours, -1, Scalar(0, 255, 255),1);
//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(),
//					Scalar(0, 255, 255), 1);
//		}
//
//		//Red
////		if(pObj->m_status == OBJ_ADDED)
////		{
//////			drawContours(imMat, contours, -1, Scalar(0, 0, 255),1);
////			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(),
////					Scalar(0, 0, 255), 1);
////		}
//	}
//
//
//	FAST_OBJECT* pFastObj;
//	for (i = 0; i < g_pFD->m_numHuman; i++)
//	{
//		pFastObj = &g_pFD->m_pHuman[i];
//		if(pFastObj->m_status != OBJ_ADDED)continue;
//
//		rectangle(imMat, pFastObj->m_boundBox.tl(), pFastObj->m_boundBox.br(),
//				Scalar(0, 0, 255), 1);
//	}
//
//	showInfo(&imMat);
//
//	g_pShow->updateFrame(&imMat);
//	*/
//}
//
//
//
//#define PUTTEXT(x,y,t) cv::putText(*pDisplayMat, String(t),Point(x, y),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1)
//
//void showInfo(Mat* pDisplayMat)
//{
//	char strBuf[512];
//	std::string strInfo;
//
//	int i;
//	int startPosH = 25;
//	int startPosV = 25;
//	int lineHeight = 20;
//	Mavlink_Messages mMsg;
//
//	i = 0;
//	mMsg = g_pMavlink->current_messages;
//
//	//Vehicle position
//	sprintf(strBuf, "Attitude: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
//			mMsg.attitude.roll, mMsg.attitude.pitch, mMsg.attitude.yaw);
//	PUTTEXT(startPosH, startPosV + lineHeight * (++i), strBuf);
//
//	sprintf(strBuf, "Speed: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
//			mMsg.attitude.rollspeed, mMsg.attitude.pitchspeed,
//			mMsg.attitude.yawspeed);
//	PUTTEXT(startPosH, startPosV + lineHeight * (++i), strBuf);
//
//	i++;
//
//	i = 0;
//	startPosH = 600;
//
//}
//
//void handleKey(int key)
//{
//	switch (key)
//	{
//	case 'q':
//		g_pUIMonitor->removeAll();
//		g_pUIMonitor->addFrame(g_pShow, 0, 0, 1980, 1080);
//		break;
//	case 'w':
//		g_pUIMonitor->removeAll();
//		g_pUIMonitor->addFrame(g_pOD->m_pContourFrame, 0,0,1980,1080);
//		break;
////	case 'e':
////		g_pUIMonitor->removeAll();
////		g_pUIMonitor->addFrame(g_pCamFront->m_pDenseFlow->m_pShowFlow, 0, 0, 1980, 1080);
////		break;
//	case 'r':
//		g_pUIMonitor->removeAll();
//		g_pUIMonitor->addFrame(g_pOD->m_pSaliencyFrame, 0, 0, 1980, 1080);
//		break;
//	case 't':
//		g_pOD->m_bOneImg = 1 - g_pOD->m_bOneImg;
//		break;
//	case 27:
//		g_bRun = false;	//ESC
//		break;
//	default:
//		break;
//	}
//}
//
//void onMouse(int event, int x, int y, int flags, void* userdata)
//{
//	fVector3 pos;
//	pos.m_x = x;
//	pos.m_y = y;
//	pos.m_z = 0;
//
//	switch (event)
//	{
//	case EVENT_LBUTTONDOWN:
//		g_pMD->setObjROI(pos);
//		break;
//	case EVENT_LBUTTONUP:
//		break;
//	case EVENT_MOUSEMOVE:
//		break;
//	case EVENT_RBUTTONDOWN:
//		break;
//	default:
//		break;
//	}
//}
//
