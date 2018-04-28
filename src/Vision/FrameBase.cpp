/*
 * FrameBase.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "../Vision/FrameBase.h"

namespace kai
{

FrameBase::FrameBase()
{
	m_tStamp = 0;
}

FrameBase::~FrameBase()
{
}

bool FrameBase::operator>(const FrameBase& f)
{
	return (this->m_tStamp > f.m_tStamp)?true:false;
}

bool FrameBase::operator>=(const FrameBase& f)
{
	return (this->m_tStamp >= f.m_tStamp)?true:false;
}

bool FrameBase::operator<(const FrameBase& f)
{
	return (this->m_tStamp < f.m_tStamp)?true:false;
}

bool FrameBase::operator<=(const FrameBase& f)
{
	return (this->m_tStamp <= f.m_tStamp)?true:false;
}

void FrameBase::operator=(const FrameBase& f)
{
	m_mat = f.m_mat;
	m_tStamp = f.m_tStamp;
}

void FrameBase::operator=(const Mat& m)
{
	m_mat = m;
	m_tStamp = getTimeUsec();
}

void FrameBase::allocate(int w, int h)
{
	m_mat = Mat::zeros(Size(w,h), CV_8UC3);
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
	fb.m_tStamp = getTimeUsec();

	return fb;
}

FrameBase FrameBase::resize(int w, int h)
{
	FrameBase fb;
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

FrameBase FrameBase::resize(double scaleW, double scaleH)
{
	Size s = this->size();
	return resize(s.width*scaleW, s.height*scaleH);
}

FrameBase FrameBase::gray(void)
{
	FrameBase fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_BGR2GRAY);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameBase FrameBase::hsv(void)
{
	FrameBase fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_BGR2HSV);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameBase FrameBase::bgra(void)
{
	FrameBase fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_BGR2BGRA);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameBase FrameBase::rgba(void)
{
	FrameBase fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_BGR2RGBA);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameBase FrameBase::f8UC3(void)
{
	FrameBase fb;
	cv::cvtColor(m_mat, fb.m_mat, CV_GRAY2BGR);
	fb.m_tStamp = m_tStamp;

	return fb;
}

FrameBase FrameBase::f32FC4(void)
{
	FrameBase fb;
	m_mat.convertTo(fb.m_mat, CV_32FC4);
	fb.m_tStamp = m_tStamp;

	return fb;
}

}

