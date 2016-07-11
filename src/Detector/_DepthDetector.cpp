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

	m_minObjArea = 0;
	m_maxObjArea = 10000000;

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
	CHECK_INFO(pJson->getVal("DEPTH_OBJDETECTOR_FPS", &FPS));
	this->setTargetFPS(FPS);

	CHECK_INFO(pJson->getVal("DEPTH_OBJDETECTOR_AREA_MIN", &m_minObjArea));
	CHECK_INFO(pJson->getVal("DEPTH_OBJDETECTOR_AREA_MAX", &m_maxObjArea));

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
	Rect bb;
	GpuMat gMat;
	GpuMat gMat2;

	if(m_pClassifier==NULL)return;
	if(m_pCamStream==NULL)return;
	m_pCam = m_pCamStream->getCameraInput();

	if(m_pCam->getType() == CAM_ZED)
	{
		gMat = *(m_pCam->getDepthFrame());
		if(gMat.empty())return;

//		cuda::divide(gMat,Scalar(50),gMat2);
//		cuda::multiply(gMat2,Scalar(50),gMat);
		cuda::threshold(gMat,gMat2,200,255,cv::THRESH_TOZERO);

		gMat2.download(m_Mat);

	}
	else
	{
		//Temporal
		return;
	}

	m_Mat.copyTo(showMat);

	// Find contours
	findContours(m_Mat, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//SIMPLE);
	// Approximate contours to polygons + get bounding rects
	vector<vector<Point> > contours_poly(contours.size());

	for (i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);

		bb = boundingRect(Mat(contours_poly[i]));
		if (bb.area() < m_minObjArea)continue;
		if (bb.area() < m_maxObjArea)continue;

		m_pClassifier->addObject(m_pCamStream->getFrame()->getCMat(), &bb, &contours_poly[i]);
	}


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

