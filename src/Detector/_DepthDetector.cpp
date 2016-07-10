/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_DepthDetector.h"

namespace kai
{
_DepthDetector::_DepthDetector()
{
	_ThreadBase();
	DetectorBase();

	m_pClassifier = NULL;
	m_pGray = NULL;

	m_pFrame = new CamFrame();
}

_DepthDetector::~_DepthDetector()
{
}

bool _DepthDetector::init(JSON* pJson, string camName)
{

	int disparity;
	CHECK_ERROR(pJson->getVal("CAM_"+camName+"_STEREO_DISPARITY", &disparity));

	double FPS = DEFAULT_FPS;
	CHECK_INFO(pJson->getVal("DEPTH_DETECTOR_FPS", &FPS));
	this->setTargetFPS(FPS);

	m_pStereo = new CamStereo();
	m_pStereo->init(disparity);

	m_pGray = new CamFrame();
	m_pDepth = new CamFrame();

	m_camFrameID = 0;

	return true;
}

bool _DepthDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in DepthDetector::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "DepthDetector.start()";

	return true;
}

void _DepthDetector::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}

}

void _DepthDetector::detect(void)
{
	int i, j;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Rect boundRect;
	Mat imMat;

	if(m_pCamStream==NULL)return;
	m_pCam = m_pCamStream->getCameraInput();

	if(m_pCam->getType() == CAM_ZED)
	{
		m_pDepth->update(m_pCam->getDepthFrame());

		if(m_pDepth->empty())return;

		m_contourMat = *(m_pDepth->getCMat());

		m_contourMat.copyTo(imMat);
	}

	// Find contours
	findContours(m_contourMat, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	//	findContours(m_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	drawContours(imMat, contours, -1, Scalar(0, 255, 0), 2);

	// Approximate contours to polygons + get bounding rects
	vector<vector<Point> > contours_poly(contours.size());

	for (i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);

		boundRect = boundingRect(Mat(contours_poly[i]));
		if (boundRect.area() < 1000)
			continue;

//		int extraW = boundRect.width * 0.15;
//		int extraH = boundRect.height * 0.15;
//
//		boundRect.x -= extraW;
//		boundRect.y -= extraH;
//		if (boundRect.x < 0)
//			boundRect.x = 0;
//		if (boundRect.y < 0)
//			boundRect.y = 0;
//
//		boundRect.width += extraW + extraW;
//		boundRect.height += extraH + extraH;
//
//		int overW = m_Mat.cols - boundRect.x - boundRect.width;
//		int overH = m_Mat.rows - boundRect.y - boundRect.height;
//		if (overW < 0)
//			boundRect.width += overW;
//		if (overH < 0)
//			boundRect.height += overH;

		rectangle( imMat, boundRect, Scalar( 0, 0, 255 ), 2 );

//		m_pClassifier->addObject(&m_Mat,&boundRect,&contours_poly[i]);


	}

	imshow("Depth",imMat);

//
//	m_pGray = m_pCamStream->getGrayFrame();
////	m_pGray = m_pCamStream->getFrame();
//	if(m_pGray==NULL)return;
//	if(m_pGray->empty())return;
//	if(m_pGray->getFrameID() <= m_camFrameID)return;
//	m_camFrameID = m_pGray->getFrameID();
//
//	m_pStereo->detect(m_pGray, m_pDepth);


}

}

