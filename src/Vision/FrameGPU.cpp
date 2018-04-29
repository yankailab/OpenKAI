/*
 * FrameGPU.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "FrameGPU.h"

#ifdef USE_CUDA

namespace kai
{

FrameGPU::FrameGPU()
{
	m_tStamp = 0;
}

FrameGPU::~FrameGPU()
{
}

uint64_t FrameGPU::tStamp(void)
{
	return m_tStamp;
}

void FrameGPU::operator=(const FrameGPU& f)
{
	m_mat = f.m_mat;
	m_tStamp = f.m_tStamp;
}

void FrameGPU::operator=(const Mat& m)
{
	m_mat = m;
	m_tStamp = getTimeUsec();
}

void FrameGPU::allocate(int w, int h)
{
	m_mat = Mat::zeros(Size(w,h), CV_8UC3);
	m_tStamp = getTimeUsec();
}

void FrameGPU::copy(const FrameGPU& f)
{
	f.m_mat.copyTo(m_mat);
	m_tStamp = f.m_tStamp;
}

void FrameGPU::copy(const Mat& m)
{
	m.copyTo(m_mat);
	m_tStamp = getTimeUsec();
}

Mat* FrameGPU::m(void)
{
	return &m_mat;
}

bool FrameGPU::bEmpty(void)
{
	return m_mat.empty();
}

Size FrameGPU::size(void)
{
	return m_mat.size();
}

FrameGPU FrameGPU::crop(Rect bb)
{
	FrameGPU fb;
	fb.m_mat = m_mat(bb);
	fb.m_tStamp = getTimeUsec();

	return fb;
}

FrameGPU FrameGPU::resize(int w, int h)
{
	FrameGPU fb;
	Size s = Size(w,h);

	if(s == this->size())
	{
		fb = m_mat;
		return fb;
	}

	cv::resize(m_mat, fb.m_mat, s);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameGPU FrameGPU::resize(double scaleW, double scaleH)
{
	Size s = this->size();
	return resize(s.width*scaleW, s.height*scaleH);
}

FrameGPU FrameGPU::gray(void)
{
	FrameGPU fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_BGR2GRAY);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameGPU FrameGPU::hsv(void)
{
	FrameGPU fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_BGR2HSV);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameGPU FrameGPU::bgra(void)
{
	FrameGPU fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_BGR2BGRA);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameGPU FrameGPU::rgba(void)
{
	FrameGPU fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_BGR2RGBA);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameGPU FrameGPU::f8UC3(void)
{
	FrameGPU fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_GRAY2BGR);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameGPU FrameGPU::f32FC4(void)
{
	FrameGPU fb;
	m_mat.convertTo(fb.m_mat, CV_32FC4);
	fb.m_tStamp = m_tStamp;

	return fb;
}

void FrameGPU::setRemap(Mat& mX, Mat& mY)
{
	m_mapX = mX;
	m_mapY = mY;
}

FrameGPU FrameGPU::remap(void)
{
	FrameGPU fb;
	cv::remap(m_mat, fb.m_mat, m_mapX, m_mapY, INTER_LINEAR);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameGPU FrameGPU::warpAffine(Mat& mWA)
{
	FrameGPU fb;
	cv::warpAffine(m_mat, fb.m_mat, mWA, m_mat.size());
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameGPU FrameGPU::flip(int iOpt)
{
	FrameGPU fb;
	cv::flip(m_mat, fb.m_mat, iOpt);
	fb.m_tStamp = m_tStamp;

	return fb;
}

}
#endif
