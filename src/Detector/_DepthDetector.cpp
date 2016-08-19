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
	m_minObjArea = 0;
	m_maxObjArea = 10000000;

	m_pFlow = NULL;
	m_pCamStream = NULL;
}

_DepthDetector::~_DepthDetector()
{
}

bool _DepthDetector::init(JSON* pJson, string camName)
{
	double FPS = DEFAULT_FPS;
	CHECK_INFO(pJson->getVal("DEPTH_OBJDETECTOR_FPS", &FPS));
	this->setTargetFPS(FPS);

	CHECK_INFO(pJson->getVal("DEPTH_OBJDETECTOR_AREA_MIN", &m_minObjArea));
	CHECK_INFO(pJson->getVal("DEPTH_OBJDETECTOR_AREA_MAX", &m_maxObjArea));

	m_pDepth = new Frame();
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
	Size imgSize;
	double scaleW;
	double scaleH;
	GpuMat gMat;
	GpuMat gMat2;
	Frame* pFrame;

	if(m_pClassifier==NULL)return;
	if(m_pCamStream==NULL)return;
	m_pCam = m_pCamStream->getCameraInput();

	if(m_pCam->getType() == CAM_ZED)
	{
		gMat = *(m_pCam->getDepthFrame());
		if(gMat.empty())return;

		//TODO: general multiple layers
//		cuda::divide(gMat,Scalar(50),gMat2);
//		cuda::multiply(gMat2,Scalar(50),gMat);

		cuda::threshold(gMat,gMat2,200,255,cv::THRESH_TOZERO);
		gMat2.download(m_Mat);
	}
	else
	{
		//Use Flow
		if(m_pFlow==NULL)return;

		pFrame = m_pFlow->m_pDepth;
		if(pFrame->empty())return;

		cuda::threshold(*pFrame->getGMat(),gMat,200,255,cv::THRESH_TOZERO);
		gMat.download(m_Mat);
	}

	imgSize = m_pCamStream->getFrame()->getSize();
	scaleW = ((double)imgSize.width)/((double)m_Mat.cols);
	scaleH = ((double)imgSize.height)/((double)m_Mat.rows);

	// Find contours
	findContours(m_Mat, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	// Approximate contours to polygons + get bounding rects
	vector<vector<Point> > contours_poly(contours.size());

	for (i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		bb = boundingRect(Mat(contours_poly[i]));

		//Convert bb area position on depth map to those on original image
		bb.x = ((double)bb.x)*scaleW;
		bb.y = ((double)bb.y)*scaleH;
		bb.width = ((double)bb.width)*scaleW-1;
		bb.height = ((double)bb.height)*scaleH-1;

		if (bb.area() < m_minObjArea)continue;
		if (bb.area() > m_maxObjArea)continue;

		m_pClassifier->addUnknownObject(m_pCamStream->getFrame()->getCMat(), &bb, &contours_poly[i]);
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

