/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_FeatureDetector.h"

namespace kai
{

_FeatureDetector::_FeatureDetector()
{
	_ThreadBase();
	DetectorBase();

	m_bThreadON = false;
	m_threadID = 0;

	m_frameID = 0;

	m_pGray = NULL;
	m_pCamStream = NULL;
	m_pGrayFrames = NULL;

	m_device = CASCADE_CPU;
	m_numCorners = 1000;

}

_FeatureDetector::~_FeatureDetector()
{
}

bool _FeatureDetector::init(string name, JSON* pJson)
{
	m_pDetector = cuda::createGoodFeaturesToTrackDetector(CV_8UC1, m_numCorners, 0.01, 0.0);
	m_pPyrLK = cuda::SparsePyrLKOpticalFlow::create();

	m_pGrayFrames = new FrameGroup();
	m_pGrayFrames->init(2);

	m_tSleep = TRD_INTERVAL_FEATUREDETECTOR;

	string targetFile;
	CHECK_ERROR(pJson->getVal("FEATURE_IMG_" + name, &targetFile));
	m_targetMat = imread(targetFile, cv::IMREAD_COLOR);

	CHECK_ERROR(pJson->getVal("FEATURE_IMG2_" + name, &targetFile));
	Mat testMat = imread(targetFile, cv::IMREAD_COLOR);

//	  cv::Mat scene1 = cv::imread(scene1_path, cv::IMREAD_COLOR);
//	  cv::Mat scene2 = cv::imread(scene2_path, cv::IMREAD_COLOR);

	m_pAkaze = cv::AKAZE::create();

	  std::vector<cv::KeyPoint> keypoint1, keypoint2;
	  m_pAkaze->detect(m_targetMat, keypoint1);
	  m_pAkaze->detect(testMat, keypoint2);

	  cv::Mat descriptor1, descriptor2;
	  m_pAkaze->compute(m_targetMat, keypoint1, descriptor1);
	  m_pAkaze->compute(testMat, keypoint2, descriptor2);

	  cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce");
	  std::vector<cv::DMatch> match, match12, match21;
	  matcher->match(descriptor1, descriptor2, match12);
	  matcher->match(descriptor2, descriptor1, match21);

	  for (size_t i = 0; i < match12.size(); i++)
	  {
	    cv::DMatch forward = match12[i];
	    cv::DMatch backward = match21[forward.trainIdx];
	    if (backward.trainIdx == forward.queryIdx)
	    {
	      match.push_back(forward);
	    }
	  }

	  cv::Mat dest;
	  cv::drawMatches(m_targetMat, keypoint1, testMat, keypoint2, match, dest);

	  imshow("AKAZE",dest);



//	string cascadeFile;
//
//	CHECK_ERROR(pJson->getVal("CASCADE_DEVICE_" + name, &m_device));
//	CHECK_ERROR(pJson->getVal("CASCADE_FILE_" + name, &cascadeFile));
//
//	if (m_device == CASCADE_CPU)
//	{
//		CHECK_ERROR(m_CC.load(cascadeFile));
//	}
//	else if (m_device == CASCADE_CUDA)
//	{
//		m_pCascade = cuda::CascadeClassifier::create(cascadeFile);
//		CHECK_ERROR(m_pCascade);
//		//TODO:set the upper limit of objects to be detected
//		//m_pCascade->
//		CHECK_INFO(pJson->getVal("CASCADE_CUDADEVICE_ID_" + name, &m_cudaDeviceID));
//	}
//
//	CHECK_INFO(pJson->getVal("CASCADE_LIFETIME_" + name, &m_objLifeTime));
//	CHECK_INFO(pJson->getVal("CASCADE_POSDIFF_" + name, &m_posDiff));
//	CHECK_ERROR(pJson->getVal("CASCADE_NUM_" + name, &m_numObj));
//
//	m_pObj = new CASCADE_OBJECT[m_numObj];
//	if (m_pObj == NULL)
//		return false;
//	m_iObj = 0;
//
//	int i;
//	for (i = 0; i < m_numObj; i++)
//	{
//		m_pObj[i].m_status = OBJ_VACANT;
//	}

	return true;
}

bool _FeatureDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in FeatureDetector::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "FeatureDetector.start()";

	return true;
}

void _FeatureDetector::update(void)
{
//	cuda::setDevice(m_cudaDeviceID);

	while (m_bThreadON)
	{
		this->updateTime();

		m_frameID = this->m_timeStamp;

		detect();

		//sleepThread can be woke up by this->wakeupThread()
		if (m_tSleep > 0)
		{
			this->sleepThread(0, m_tSleep);
		}
	}

}

void _FeatureDetector::detect(void)
{
	GpuMat d_prevPts;
	GpuMat d_nextPts;
	GpuMat d_status;
	CamFrame* pGray;
	CamFrame* pNextFrame;
	CamFrame* pPrevFrame;
	Mat   dMat, tMat;
	vector<Point2f> prevPts;
	vector<Point2f> nextPts;
	vector<uchar> status;
	int i;
	int numPts;
	int numV;
	cv::Point p, q;

	//http://docs.opencv.org/2.4/doc/tutorials/features2d/feature_homography/feature_homography.html

	if(m_pCamStream==NULL)return;

	pGray = m_pCamStream->getGrayFrame();
	if(pGray->empty())return;

	pNextFrame = m_pGrayFrames->getLastFrame();
	if(pGray->getFrameID() <= pNextFrame->getFrameID())return;

	m_pGrayFrames->updateFrameIndex();
	pNextFrame = m_pGrayFrames->getLastFrame();
	pPrevFrame = m_pGrayFrames->getPrevFrame();

//	pNextFrame->getResizedOf(pGray,m_width,m_height);
	GpuMat* pPrev = pPrevFrame->getGMat();
	GpuMat* pNext = pNextFrame->getGMat();

	if(pPrev->empty())return;
	if(pNext->empty())return;

	m_pDetector->detect(*pPrev, d_prevPts);
	m_pPyrLK->calc(
		*pPrev,
		*pNext,
		d_prevPts,
		d_nextPts,
		d_status);

	prevPts.resize(d_prevPts.cols);
	tMat = Mat(1, d_prevPts.cols, CV_32FC2, (void*)&prevPts[0]);
	d_prevPts.download(tMat);

	nextPts.resize(d_nextPts.cols);
	tMat = Mat(1, d_nextPts.cols, CV_32FC2, (void*)&nextPts[0]);
	d_nextPts.download(tMat);

	status.resize(d_status.cols);
	tMat = Mat(1, d_status.cols, CV_8UC1, (void*)&status[0]);
	d_status.download(tMat);

//	//Find the flow direction
//	m_flow.m_x = 0;
//	m_flow.m_y = 0;
//	m_flow.m_z = 0;
//	m_flow.m_w = 0;
//
//	absXtot = 0.0;
//	absYtot = 0.0;
//	numPts = prevPts.size();
//	numV = 0;
//
//	for ( i = 0; i < numPts; i++)
//	{
//		if (!status[i])continue;
//
//		p = prevPts[i];
//		q = nextPts[i];
//
//		dX = q.x - p.x;
//		dY = q.y - p.y;
//		distSQ = dX*dX + dY*dY;
//
//		if (distSQ < distSQMin)continue;
//		if (distSQ > distSQMax)continue;
//
//		m_flow.m_x += dX;
//		m_flow.m_y += dY;
//		absXtot += abs(dX);
//		absYtot += abs(dY);
//		numV++;
//	}
//
//	//Calc the motion vector
//	vBase = 1.0/(double)numV;
//	distSQTotal = absXtot*absXtot + absYtot*absYtot;
//
//	m_flow.m_x *= vBase;
//	m_flow.m_y *= vBase;
//	m_flow.m_z = 0;	//TODO: Change in Height
//	m_flow.m_w = 0;	//TODO: Rotation
//
//	return m_flow;

//#if (SHOW_FEATURE_FLOW==1)
//	m_Mat.m_pNew->download(dMat);
//	p = Point(mSize.width / 2, mSize.height / 2);
//	q = p + Point(m_flow.m_x, m_flow.m_y);
//	line(dMat, p, q, Scalar(0,255,0), 3);
////	drawFeatureFlow(dMat, prevPts, nextPts, status, Scalar(255, 0, 0));
//	imshow(CF_WINDOW, dMat);
//#endif
//
//#else
//
//	m_pDetector->detect(m_Mat.m_uFrame, m_keypoint1);
//	m_pDetector->detect(m_Mat.m_uFrame, m_keypoint2);
//
//	m_pExtractor->compute(m_Mat.m_uFrame, m_keypoint1, m_descriptor1);
//	m_pExtractor->compute(m_Mat.m_uFrame, m_keypoint2, m_descriptor2);
//
//	m_pMatcher->match(m_descriptor1, m_descriptor2, m_dMatch);
//#endif
}


}

