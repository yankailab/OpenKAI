/*
 * CameraOpticalFlow.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_DenseFlow.h"

#include "../Base/common.h"
#include "stdio.h"
#include "../Base/cvplatform.h"

namespace kai
{

_DenseFlow::_DenseFlow()
{
	_ThreadBase();

	m_width = 640;
	m_height = 480;

	m_pFlowFrame = NULL;
	m_pShowFlow = NULL;

}

_DenseFlow::~_DenseFlow()
{
}

bool _DenseFlow::init(JSON* pJson, string camName)
{
	CHECK_INFO(pJson->getVal("DENSEFLOW_"+camName+"_WIDTH", &m_width));
	CHECK_INFO(pJson->getVal("DENSEFLOW_"+camName+"_HEIGHT", &m_height));

	//	m_flowMat = GpuMat(SMALL_WIDTH, SMALL_HEIGHT, CV_32FC2);
	m_pFarn = cuda::FarnebackOpticalFlow::create();

	m_pFlowFrame = new CamFrame();
	m_pShowFlow = new CamFrame();

	return true;
}


bool _DenseFlow::start(void)
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

void _DenseFlow::update(void)
{
	m_tSleep = TRD_INTERVAL_DENSEFLOW;

	while (m_bThreadON)
	{
		this->updateTime();

		detect();

		//sleepThread can be woke up by this->wakeupThread()
		this->sleepThread(0, m_tSleep);
	}

}

void _DenseFlow::updateFrame(CamFrame* pFrame)
{
	if(pFrame==NULL)return;
	if(pFrame->getCurrentFrame()->empty())return;

	m_pFlowFrame->switchFrame();
	pFrame->getResized(640,480, m_pFlowFrame);

	this->wakeupThread();
}

fVector4 _DenseFlow::detect(void)
{
	int i, j;
	cv::Point2f vFlow;
	double base;
	GpuMat* pPrev;
	GpuMat* pNext;

	m_flow.m_x = 0;
	m_flow.m_y = 0;
	m_flow.m_z = 0;
	m_flow.m_w = 0;

	pPrev = m_pFlowFrame->getPreviousFrame();
	pNext = m_pFlowFrame->getCurrentFrame();

	if(pNext->empty())return m_flow;
	if(pPrev->size() != pNext->size())return m_flow;

	m_pFarn->calc(*pPrev, *pNext, m_GFlowMat);

	m_GFlowMat.download(m_uFlowMat);

	for (i = 0; i < m_uFlowMat.rows; i++)
	{
		for (j = 0; j < m_uFlowMat.cols; j++)
		{
			vFlow = m_uFlowMat.at<cv::Point2f>(i, j);
			m_flow.m_x += vFlow.x;
			m_flow.m_y += vFlow.y;
		}
	}

	base = 10.0/(double)(m_uFlowMat.rows * m_uFlowMat.cols);
	m_flow.m_x *= base;
	m_flow.m_y *= base;

	generateFlowMap(m_GFlowMat);

	return m_flow;

/*	cv::Size mSize = m_uFlowMat.size();
	Point p = Point(mSize.width / 2, mSize.height / 2);
	Point q = p + Point(m_flow.m_x, m_flow.m_y);
	line(m_uFlowMat, p, q, Scalar(0, 255, 0), 3);
	imshow("flow", m_uFlowMat);
*/

}



inline bool _DenseFlow::isFlowCorrect(Point2f u)
{
	return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}

Vec3b _DenseFlow::computeColor(float fx, float fy)
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

void _DenseFlow::drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion)
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

void _DenseFlow::generateFlowMap(const GpuMat& d_flow)
{
	GpuMat planes[2];
	cuda::split(d_flow, planes);

	Mat flowx(planes[0]);
	Mat flowy(planes[1]);

	Mat out;
	drawOpticalFlow(flowx, flowy, out, 10);

	m_pShowFlow->updateFrame(&out);
}

} /* namespace kai */
