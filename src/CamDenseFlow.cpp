/*
 * CameraOpticalFlow.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "CamDenseFlow.h"

#include "common.h"
#include "stdio.h"
#include "cvplatform.h"

namespace kai
{

CamDenseFlow::CamDenseFlow()
{
	// TODO Auto-generated constructor stub

}

CamDenseFlow::~CamDenseFlow()
{
	// TODO Auto-generated destructor stub
}

bool CamDenseFlow::init(void)
{
//	m_flowMat = GpuMat(SMALL_WIDTH, SMALL_HEIGHT, CV_32FC2);
	m_pFarn = cuda::FarnebackOpticalFlow::create();

	return true;
}

fVector4 CamDenseFlow::detect(CamFrame* pFrame)
{
	int i, j;
	cv::Point2f vFlow;
	double base;

	m_flow.m_x = 0;
	m_flow.m_y = 0;
	m_flow.m_z = 0;
	m_flow.m_w = 0;

	if(pFrame->m_pPrev->empty())return m_flow;
	if(pFrame->m_pPrev->size()!=pFrame->m_pNext->size())return m_flow;
	m_pFarn->calc(*(pFrame->m_pPrev), *(pFrame->m_pNext), m_flowMat);

	m_flowMat.download(m_uFlowMat);

//	imshow("DenseFlow",m_flowY);








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

	return m_flow;

#if (SHOW_OPTICAL_FLOW == 1)
	m_Mat.m_pNew->download(m_uFlowMat);
	cv::Size mSize = m_uFlowMat.size();
	Point p = Point(mSize.width / 2, mSize.height / 2);
	Point q = p + Point(m_flow.m_x, m_flow.m_y);
	line(m_uFlowMat, p, q, Scalar(0, 255, 0), 3);
	imshow(OPF_WINDOW, m_uFlowMat);
//	showFlow(OPF_WINDOW, m_flowMat);
#endif

}



inline bool CamDenseFlow::isFlowCorrect(Point2f u)
{
	return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}

Vec3b CamDenseFlow::computeColor(float fx, float fy)
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

void CamDenseFlow::drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion)
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

void CamDenseFlow::showFlow(const char* name, const GpuMat& d_flow)
{
	GpuMat planes[2];
	cuda::split(d_flow, planes);

	Mat flowx(planes[0]);
	Mat flowy(planes[1]);

	Mat out;
	drawOpticalFlow(flowx, flowy, out, 10);

	imshow(name, out);
}

} /* namespace kai */
