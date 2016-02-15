/*
 * CameraFeatureFlow.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "CamSparseFlow.h"

namespace kai
{

CamSparseFlow::CamSparseFlow()
{
	m_flow.m_x = 0;
	m_flow.m_y = 0;
	m_flow.m_z = 0;
	m_flow.m_w = 0;

	m_diffPrev.m_x = 0;
	m_diffPrev.m_y = 0;
	m_diffPrev.m_z = 0;

	m_numCorners = 1000;
}

CamSparseFlow::~CamSparseFlow()
{
	// TODO Auto-generated destructor stub
}


bool CamSparseFlow::init(void)
{

#ifdef USE_CUDA
	m_pDetector = cuda::createGoodFeaturesToTrackDetector(CV_8UC1, m_numCorners, 0.01, 0.0);
	m_pPyrLK = cuda::SparsePyrLKOpticalFlow::create();

#else
	//	m_pDetector = FeatureDetector::create("AKAZE");
	//	m_pExtractor = DescriptorExtractor::create("AKAZE");
	//	m_pMatcher = DescriptorMatcher::create("BruteForce-Hamming");
#endif

	return true;
}

fVector4 CamSparseFlow::detect(CamFrame* pFrame)
{
#ifdef USE_CUDA
	GpuMat d_prevPts;
	GpuMat d_nextPts;
	GpuMat d_status;
	Mat   dMat, tMat;
	vector<Point2f> prevPts;
	vector<Point2f> nextPts;
	vector<uchar> status;
	int i;
	int numPts;
	int numV;
	cv::Point p, q;
	double dX, dY;
	double distSQ, distSQTotal, absXtot, absYtot;
	double distSQMin;
	double distSQMax;
	double vBase;

	GpuMat* pPrev = pFrame->getPrevious();
	GpuMat* pNext = pFrame->getCurrent();

	distSQMin = 1;
	distSQMax = (pNext->cols + pNext->rows)*0.15;
	distSQMax *= distSQMax;

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

	//Find the flow direction
	m_flow.m_x = 0;
	m_flow.m_y = 0;
	m_flow.m_z = 0;
	m_flow.m_w = 0;

	absXtot = 0.0;
	absYtot = 0.0;
	numPts = prevPts.size();
	numV = 0;

	for ( i = 0; i < numPts; i++)
	{
		if (!status[i])continue;

		p = prevPts[i];
		q = nextPts[i];

		dX = q.x - p.x;
		dY = q.y - p.y;
		distSQ = dX*dX + dY*dY;

		if (distSQ < distSQMin)continue;
		if (distSQ > distSQMax)continue;

		m_flow.m_x += dX;
		m_flow.m_y += dY;
		absXtot += abs(dX);
		absYtot += abs(dY);
		numV++;
	}

	//Calc the motion vector
	vBase = 1.0/(double)numV;
	distSQTotal = absXtot*absXtot + absYtot*absYtot;

	m_flow.m_x *= vBase;
	m_flow.m_y *= vBase;
	m_flow.m_z = 0;	//TODO: Change in Height
	m_flow.m_w = 0;	//TODO: Rotation

	return m_flow;

#if (SHOW_FEATURE_FLOW==1)
	m_Mat.m_pNew->download(dMat);
	p = Point(mSize.width / 2, mSize.height / 2);
	q = p + Point(m_flow.m_x, m_flow.m_y);
	line(dMat, p, q, Scalar(0,255,0), 3);
//	drawFeatureFlow(dMat, prevPts, nextPts, status, Scalar(255, 0, 0));
	imshow(CF_WINDOW, dMat);
#endif

#else

	m_pDetector->detect(m_Mat.m_uFrame, m_keypoint1);
	m_pDetector->detect(m_Mat.m_uFrame, m_keypoint2);

	m_pExtractor->compute(m_Mat.m_uFrame, m_keypoint1, m_descriptor1);
	m_pExtractor->compute(m_Mat.m_uFrame, m_keypoint2, m_descriptor2);

	m_pMatcher->match(m_descriptor1, m_descriptor2, m_dMatch);
#endif
}


} /* namespace kai */
