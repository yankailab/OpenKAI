/*
 * CameraInput.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "Frame.h"

namespace kai
{

Frame::Frame()
{
	//Common
	m_CMat.m_frameID = 0;

#ifdef USE_CUDA
	m_GMat.m_frameID = 0;
#endif

}

Frame::~Frame()
{
}

void Frame::allocate(int w, int h)
{
	m_CMat.m_mat = Mat::zeros(Size(w,h), CV_8UC3);
	updatedCMat();
}

void Frame::getResizedOf(Frame* pFrom, int width, int height)
{
	NULL_(pFrom);
	IF_(pFrom->empty());

	cv::Size newSize = cv::Size(width,height);

	if(newSize == pFrom->getSize())
	{
		this->update(pFrom);
	}
	else
	{
#ifdef USE_CUDA

#ifdef USE_OPENCV4TEGRA
		gpu::resize(*pFrom->getGMat(), m_GMat.m_mat, newSize);
#else
		cuda::resize(*pFrom->getGMat(), m_GMat.m_mat, newSize);
#endif
		updatedGMat();

#elif defined USE_OPENCL

#else
		cv::resize(*pFrom->getNextCMat(), m_CMat.m_mat, newSize);
		updatedCMat();
#endif

	}

}

void Frame::getGrayOf(Frame* pFrom)
{
	NULL_(pFrom);

#ifdef USE_CUDA
	IF_(pFrom->getGMat()->channels()!=3);

#ifdef USE_OPENCV4TEGRA
	gpu::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2GRAY);
#else
	cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2GRAY);
#endif

	updatedGMat();
#elif defined USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_BGR2GRAY);
	updatedCMat();
#endif

}

void Frame::getHSVOf(Frame* pFrom)
{
	NULL_(pFrom);

#ifdef USE_CUDA
	//RGB or BGR depends on device
	IF_(pFrom->getGMat()->channels()!=3);

#ifdef USE_OPENCV4TEGRA
	gpu::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2HSV);
#else
	cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2HSV);
#endif

	updatedGMat();
#elif defined USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_BGR2HSV);
	updatedCMat();
#endif

}

void Frame::getBGRAOf(Frame* pFrom)
{
	NULL_(pFrom);

#ifdef USE_CUDA
	IF_(pFrom->getGMat()->channels()!=3);

#ifdef USE_OPENCV4TEGRA
	gpu::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2BGRA);
#else
	cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2BGRA);
#endif

	updatedGMat();
#elif defined USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_BGR2BGRA);
	updatedCMat();
#endif

}

void Frame::getRGBAOf(Frame* pFrom)
{
	NULL_(pFrom);

#ifdef USE_CUDA
	IF_(pFrom->getGMat()->channels()!=3);

#ifdef USE_OPENCV4TEGRA
	gpu::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2RGBA);
#else
	cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2RGBA);
#endif

	updatedGMat();
#elif defined USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_BGR2RGBA);
	updatedCMat();
#endif

}

void Frame::get8UC3Of(Frame* pFrom)
{
	NULL_(pFrom);

#ifdef USE_CUDA
	if(pFrom->getGMat()->type()==CV_8UC3)
	{
		pFrom->getGMat()->copyTo(m_GMat.m_mat);
	}
	else
	{
		if(pFrom->getGMat()->channels()!=1)return;

#ifdef USE_OPENCV4TEGRA
		gpu::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_GRAY2BGR);
#else
		cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_GRAY2BGR);
#endif
	}

	updatedGMat();
#elif defined USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_GRAY2BGR);
	updatedCMat();
#endif

}

void Frame::get32FC4Of(Frame* pFrom)
{
	NULL_(pFrom);

#ifdef USE_CUDA
	pFrom->getGMat()->convertTo(m_GMat.m_mat, CV_32FC4);

	updatedGMat();
#elif defined USE_OPENCL

#else
	if(pFrom->getGMat()->channels()!=4)return;
	pFrom->getNextCMat()->convertTo(m_CMat.m_mat, CV_32FC4);
	updatedCMat();
#endif

}

uint64_t Frame::getFrameID(void)
{
	uint64_t frameID = m_CMat.m_frameID;

#ifdef USE_CUDA
	if(frameID < m_GMat.m_frameID)frameID = m_GMat.m_frameID;
#elif defined USE_OPENCL

#endif

	return frameID;
}

bool Frame::empty(void)
{
#ifdef USE_CUDA
	if(m_CMat.m_frameID > m_GMat.m_frameID)
	{
		return m_CMat.m_mat.empty();
	}

	return m_GMat.m_mat.empty();

#elif defined USE_OPENCL

#else
	return m_CMat.m_mat.empty();
#endif

}

bool Frame::isNewerThan(Frame* pFrame)
{
	NULL_F(pFrame);
	if(pFrame->getFrameID() < this->getFrameID())
	{
		return true;
	}

	return false;
}

void Frame::update(Frame* pFrame)
{
	NULL_(pFrame);

#ifdef USE_CUDA
	pFrame->getGMat()->copyTo(m_GMat.m_mat);
	updatedGMat();
#elif defined USE_OPENCL

#else
	pFrame->getCMat()->copyTo(m_CMat.m_mat);
	updatedCMat();
#endif
}

#ifdef USE_CUDA
void Frame::update(GpuMat* pGpuFrame)
{
	NULL_(pGpuFrame);

	pGpuFrame->copyTo(m_GMat.m_mat);
	updatedGMat();
}
#endif

void Frame::update(Mat* pFrame)
{
	NULL_(pFrame);

#ifdef USE_CUDA
	pFrame->copyTo(m_CMat.m_mat);
	updatedCMat();
#elif defined USE_OPENCL

#else
	pFrame->copyTo(m_CMat.m_mat);
	updatedCMat();
#endif
}

inline void Frame::updatedCMat(void)
{
	m_CMat.m_frameID = get_time_usec();
}

#ifdef USE_CUDA
void Frame::updatedGMat(void)
{
	m_GMat.m_frameID = get_time_usec();
}
#endif

#ifdef USE_CUDA
GpuMat* Frame::getGMat(void)
{
	//Return the latest content
	if(m_CMat.m_frameID > m_GMat.m_frameID)
	{
		m_GMat.m_mat.upload(m_CMat.m_mat);
		m_GMat.m_frameID = m_CMat.m_frameID;
	}

	return &m_GMat.m_mat;
}
#endif

Mat* Frame::getCMat(void)
{
#ifdef USE_CUDA
	//Return the latest content
	if(m_GMat.m_frameID > m_CMat.m_frameID)
	{
		m_GMat.m_mat.download(m_CMat.m_mat);
		m_CMat.m_frameID = m_GMat.m_frameID;
	}
#endif

	return &m_CMat.m_mat;
}

Size Frame::getSize(void)
{
	Size mySize = Size(0,0);
	mySize = m_CMat.m_mat.size();

#ifdef USE_CUDA
	if(m_GMat.m_frameID > m_CMat.m_frameID)
	{
		mySize = m_GMat.m_mat.size();
	}
#elif defined USE_OPENCL

#endif

	return mySize;
}

}

