/*
 * FrameBase.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "FrameBase.h"

namespace kai
{

FrameBase::FrameBase()
{
	m_tStamp = 0;
}

FrameBase::~FrameBase()
{
}

uint64_t FrameBase::tStamp(void)
{
	return m_tStamp;
}

void FrameBase::operator=(const Mat& m)
{
	m_mat = m;
	m_tStamp = getTimeUsec();
}

void FrameBase::allocate(int w, int h)
{
	m_mat = Mat(h, w, CV_8UC3, Scalar(0,0,0));
	m_tStamp = getTimeUsec();
}

void FrameBase::copy(const FrameBase& f)
{
	f.m_mat.copyTo(m_mat);
	m_tStamp = f.m_tStamp;
}

void FrameBase::copy(const Mat& m)
{
	m.copyTo(m_mat);
	m_tStamp = getTimeUsec();
}

Mat* FrameBase::m(void)
{
	return &m_mat;
}

bool FrameBase::bEmpty(void)
{
	return m_mat.empty();
}

Size FrameBase::size(void)
{
	return m_mat.size();
}

FrameBase FrameBase::crop(Rect bb)
{
	FrameBase fb;
	fb.m_mat = m_mat(bb);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameBase FrameBase::resize(int w, int h)
{
	FrameBase fb;
	fb.m_tStamp = m_tStamp;

	Size s = Size(w,h);
	if(s == this->size())
		fb.m_mat = m_mat;
	else
		cv::resize(m_mat, fb.m_mat, s);

	return fb;
}

FrameBase FrameBase::resize(double scaleW, double scaleH)
{
	Size s = this->size();
	int sw = s.width*scaleW;
	int sh = s.height*scaleH;
	return resize(sw, sh);
}

FrameBase FrameBase::cvtTo(int rType)
{
	FrameBase fb;
	fb.m_tStamp = m_tStamp;
	m_mat.convertTo(fb.m_mat, CV_MAKETYPE(rType,(m_mat.channels())));

	return fb;
}

FrameBase FrameBase::cvtColor(int code)
{
	FrameBase fb;
	fb.m_tStamp = m_tStamp;
	cv::cvtColor(m_mat, fb.m_mat, code);

	return fb;
}

void FrameBase::setRemap(Mat& mX, Mat& mY)
{
	m_mapX = mX;
	m_mapY = mY;
}

FrameBase FrameBase::remap(void)
{
	FrameBase fb;
	fb.m_tStamp = m_tStamp;
	cv::remap(m_mat, fb.m_mat, m_mapX, m_mapY, INTER_LINEAR);

	return fb;
}

FrameBase FrameBase::warpAffine(Mat& mWA)
{
	FrameBase fb;
	fb.m_tStamp = m_tStamp;
	cv::warpAffine(m_mat, fb.m_mat, mWA, m_mat.size());

	return fb;
}

FrameBase FrameBase::flip(int iOpt)
{
	FrameBase fb;
	fb.m_tStamp = m_tStamp;
	cv::flip(m_mat, fb.m_mat, iOpt);

	return fb;
}

}
