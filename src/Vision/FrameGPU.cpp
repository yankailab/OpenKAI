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
	m_tStampG = 0;
}

FrameGPU::~FrameGPU()
{
}

uint64_t FrameGPU::tStamp(void)
{
	return (m_tStamp > m_tStampG) ? m_tStamp : m_tStampG;
}

void FrameGPU::updateTstamp(uint64_t t)
{
	m_tStamp = t;
	m_tStampG = 0;
}

void FrameGPU::updateTstampG(uint64_t t)
{
	m_tStamp = 0;
	m_tStampG = t;
}

void FrameGPU::operator=(const Mat& m)
{
	m_mat = m;
	updateTstamp(getTimeUsec());
}

void FrameGPU::operator=(const GpuMat& m)
{
	m_matG = m;
	updateTstampG(getTimeUsec());
}

void FrameGPU::allocate(int w, int h)
{
	m_mat = Mat::zeros(Size(w,h), CV_8UC3);
	updateTstamp(getTimeUsec());
}

void FrameGPU::copy(const FrameGPU& f)
{
	if(f.m_tStamp > f.m_tStampG)
	{
		f.m_mat.copyTo(m_mat);
		updateTstamp(f.m_tStamp);
	}
	else
	{
		f.m_matG.copyTo(m_matG);
		updateTstampG(f.m_tStampG);
	}
}

void FrameGPU::copy(const Mat& m)
{
	m.copyTo(m_mat);
	updateTstamp(getTimeUsec());
}

void FrameGPU::copy(const GpuMat& m)
{
	m.copyTo(m_matG);
	updateTstampG(getTimeUsec());
}

Mat* FrameGPU::m(void)
{
	update();
	return &m_mat;
}

GpuMat* FrameGPU::gm(void)
{
	updateG();
	return &m_matG;
}

bool FrameGPU::bEmpty(void)
{
	if(m_tStamp > m_tStampG)return m_mat.empty();

	return m_matG.empty();
}

Size FrameGPU::size(void)
{
	if(m_tStamp > m_tStampG)return m_mat.size();

	return m_matG.size();
}

FrameGPU FrameGPU::crop(Rect bb)
{
	FrameGPU fb;

	if(m_tStamp > m_tStampG)
	{
		fb.m_mat = m_mat(bb);
		fb.updateTstamp(m_tStamp);
	}
	else
	{
		fb.m_matG = m_matG(bb);
		fb.updateTstampG(m_tStampG);
	}

	return fb;
}

FrameGPU FrameGPU::resize(int w, int h)
{
	updateG();

	FrameGPU fb;
	cuda::resize(m_matG, fb.m_matG, Size(w,h));
	fb.updateTstampG(m_tStampG);

	return fb;
}

FrameGPU FrameGPU::resize(double scaleW, double scaleH)
{
	Size s = this->size();
	int sw = s.width*scaleW;
	int sh = s.height*scaleH;
	return resize(sw, sh);
}

FrameGPU FrameGPU::cvtTo(int rType)
{
	updateG();

	FrameGPU fb;
	m_matG.convertTo(fb.m_matG, CV_MAKETYPE(rType,(m_matG.channels())));
	fb.updateTstampG(m_tStampG);

	return fb;
}

FrameGPU FrameGPU::cvtColor(int code)
{
	updateG();

	FrameGPU fb;
	cuda::cvtColor(m_matG, fb.m_matG, code);
	fb.updateTstampG(m_tStampG);

	return fb;
}

void FrameGPU::setRemap(Mat& mX, Mat& mY)
{
	m_mapX.upload(mX);
	m_mapY.upload(mY);
}

FrameGPU FrameGPU::remap(void)
{
	updateG();

	FrameGPU fb;
	cuda::remap(m_matG, fb.m_matG, m_mapX, m_mapY, INTER_LINEAR);
	fb.updateTstampG(m_tStampG);

	return fb;
}

FrameGPU FrameGPU::warpAffine(Mat& mWA)
{
	updateG();

	FrameGPU fb;
	cuda::warpAffine(m_matG, fb.m_matG, mWA, m_matG.size());
	fb.updateTstampG(m_tStampG);

	return fb;
}

FrameGPU FrameGPU::flip(int iOpt)
{
	updateG();

	FrameGPU fb;
	cuda::flip(m_matG, fb.m_matG, iOpt);
	fb.updateTstampG(m_tStampG);

	return fb;
}

void FrameGPU::update(void)
{
	IF_(m_tStamp >= m_tStampG);

	m_matG.download(m_mat);
	m_tStamp = m_tStampG;
}

void FrameGPU::updateG(void)
{
	IF_(m_tStamp <= m_tStampG);

	m_matG.upload(m_mat);
	m_tStampG = m_tStamp;
}

void FrameGPU::sync(void)
{
	update();
	updateG();
}

}
#endif
