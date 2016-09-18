/*
 * CameraOpticalFlow.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_Flow.h"
#include "../Base/common.h"
#include "stdio.h"
#include "../Base/cvplatform.h"

namespace kai
{

_Flow::_Flow()
{
	_ThreadBase();

	m_width = 640;
	m_height = 480;

	m_pCamStream = NULL;
	m_pGrayFrames = NULL;

	m_flowMax = 100;
	m_flowAvr = 0.0;
	m_pDepth = 0;

}

_Flow::~_Flow()
{
}

bool _Flow::init(Config* pConfig)
{
	if (this->_ThreadBase::init(pConfig)==false)
		return false;

	string presetDir = "";
	string labelFile;

	F_INFO_(pConfig->o("APP")->v("presetDir", &presetDir));

	F_INFO_(pConfig->v("bDepth", &m_bDepth));
	F_INFO_(pConfig->v("width", &m_width));
	F_INFO_(pConfig->v("height", &m_height));
	F_INFO_(pConfig->v("flowMax", &m_flowMax));
	F_INFO_(pConfig->v("colorFile", &labelFile));

	m_pDepth = new Frame();
	m_pFarn = cuda::FarnebackOpticalFlow::create();

	m_pGrayFrames = new FrameGroup();
	m_pGrayFrames->init(2);

	m_GDMat = GpuMat(m_height, m_width, CV_32FC1, Scalar(0));

	return true;

	//	m_labelColor = imread(presetDir+labelFile, 1);
	//	m_pGpuLUT = cuda::createLookUpTable(m_labelColor);
	//	m_pSeg = new CamFrame();
	//	m_flowMat = GpuMat(SMALL_WIDTH, SMALL_HEIGHT, CV_32FC2);
}

bool _Flow::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Flow::update(void)
{

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}

}

void _Flow::detect(void)
{
	Frame* pGray;
	Frame* pNextFrame;
	Frame* pPrevFrame;
	GpuMat* pPrev;
	GpuMat* pNext;
	GpuMat GMat;
	GpuMat pGMat[2];

	if(m_pCamStream==NULL)return;

	pGray = m_pCamStream->getGrayFrame();
	if(pGray->empty())return;

	pNextFrame = m_pGrayFrames->getLastFrame();
	if(pGray->getFrameID() <= pNextFrame->getFrameID())return;

	m_pGrayFrames->updateFrameIndex();
	pNextFrame = m_pGrayFrames->getLastFrame();
	pPrevFrame = m_pGrayFrames->getPrevFrame();

	pNextFrame->getResizedOf(pGray,m_width,m_height);
	pPrev = pPrevFrame->getGMat();
	pNext = pNextFrame->getGMat();

	if(pPrev->empty())return;
	if(pNext->empty())return;
	if(pPrev->size() != pNext->size())return;

	m_pFarn->calc(*pPrev, *pNext, m_GFlowMat);



	//Generate Depth Map from Flow
	if(m_bDepth==0)return;

	cuda::abs(m_GFlowMat, GMat);
	cuda::split(GMat, pGMat);
	cuda::add(pGMat[0],pGMat[1], GMat);
	cuda::multiply(GMat, Scalar(100), pGMat[1]);

	pGMat[1].convertTo(*(m_pDepth->getGMat()),CV_8UC1);
	m_pDepth->updatedGMat();

//	m_flowMax = cuda::sum(fGMat)[0] / (fGMat.cols*fGMat.rows);
//	fInterval = 1.0/m_flowMax;
//	fInterval *= 50.0;

//	cuda::min(fGMat,Scalar(m_flowMax),pGMat[0]);
//	cuda::multiply(pGMat[0],Scalar(fInterval),fGMat);

//	cv::cuda::cvtColor(depthGMat, idxGMat, CV_GRAY2BGR);
//	m_pGpuLUT->transform(idxGMat,segGMat);

}



inline bool _Flow::isFlowCorrect(Point2f u)
{
	return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}

Vec3b _Flow::computeColor(float fx, float fy)
{
	static bool first = true;

	// relative lengths of color transitions:
	// these are chosen based on perceptual similarity
	// (e.g. one can distinguish more shades between red and yellow
	//  than between yellow and green)
	const int RY = 15;
	const int YG = 6;
	const int GC = 4;
	const int CB = 11;
	const int BM = 13;
	const int MR = 6;
	const int NCOLS = RY + YG + GC + CB + BM + MR;
	static Vec3i colorWheel[NCOLS];

	if (first)
	{
		int k = 0;

		for (int i = 0; i < RY; ++i, ++k)
			colorWheel[k] = Vec3i(255, 255 * i / RY, 0);

		for (int i = 0; i < YG; ++i, ++k)
			colorWheel[k] = Vec3i(255 - 255 * i / YG, 255, 0);

		for (int i = 0; i < GC; ++i, ++k)
			colorWheel[k] = Vec3i(0, 255, 255 * i / GC);

		for (int i = 0; i < CB; ++i, ++k)
			colorWheel[k] = Vec3i(0, 255 - 255 * i / CB, 255);

		for (int i = 0; i < BM; ++i, ++k)
			colorWheel[k] = Vec3i(255 * i / BM, 0, 255);

		for (int i = 0; i < MR; ++i, ++k)
			colorWheel[k] = Vec3i(255, 0, 255 - 255 * i / MR);

		first = false;
	}

	const float rad = sqrt(fx * fx + fy * fy);
	const float a = atan2(-fy, -fx) / (float)CV_PI;

	const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
	const int k0 = static_cast<int>(fk);
	const int k1 = (k0 + 1) % NCOLS;
	const float f = fk - k0;

	Vec3b pix;

	for (int b = 0; b < 3; b++)
	{
		const float col0 = colorWheel[k0][b] / 255.0f;
		const float col1 = colorWheel[k1][b] / 255.0f;

		float col = (1 - f) * col0 + f * col1;

		if (rad <= 1)
			col = 1 - rad * (1 - col); // increase saturation with radius
		else
			col *= .75; // out of range

		pix[2 - b] = static_cast<uchar>(255.0 * col);
	}

	return pix;
}

void _Flow::drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion)
{
	dst.create(flowx.size(), CV_8UC3);
	dst.setTo(Scalar::all(0));

	// determine motion range:
	float maxrad = maxmotion;

	if (maxmotion <= 0)
	{
		maxrad = 1;
		for (int y = 0; y < flowx.rows; ++y)
		{
			for (int x = 0; x < flowx.cols; ++x)
			{
				Point2f u(flowx(y, x), flowy(y, x));

				if (!isFlowCorrect(u))
					continue;

				maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
			}
		}
	}

	for (int y = 0; y < flowx.rows; ++y)
	{
		for (int x = 0; x < flowx.cols; ++x)
		{
			Point2f u(flowx(y, x), flowy(y, x));

			if (isFlowCorrect(u))
				dst.at<Vec3b>(y, x) = computeColor(u.x / maxrad, u.y / maxrad);
		}
	}
}

void _Flow::generateFlowMap(const GpuMat& d_flow)
{
	GpuMat planes[2];
	cuda::split(d_flow, planes);

	Mat flowx(planes[0]);
	Mat flowy(planes[1]);

	Mat out;
	drawOpticalFlow(flowx, flowy, out, 10);

//	out.copyTo(m_cMat);
}

} /* namespace kai */
