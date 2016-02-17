/*
 * CameraInput.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "CamFrame.h"

namespace kai
{

CamFrame::CamFrame()
{
	//Common
	m_CMat.m_frameID = 0;

#ifdef USE_CUDA
	m_GMat.m_frameID = 0;
#endif

}

CamFrame::~CamFrame()
{
}

void CamFrame::getResizedOf(CamFrame* pFrom, int width, int height)
{
	if(!pFrom)return;
	if(pFrom->empty())return;

	cv::Size newSize = cv::Size(width,height);

	if(newSize == pFrom->getSize())//pFrom->getNextGMat()->size())
	{
		this->update(pFrom);
	}
	else
	{
#ifdef USE_CUDA
		cuda::resize(*pFrom->getGMat(), m_GMat.m_mat, newSize);
		updatedGMat();
#elif USE_OPENCL

#else
		cv::resize(*pFrom->getNextCMat(), m_CMat.m_mat, newSize);
		updatedCMat();
#endif

	}

}

void CamFrame::getGrayOf(CamFrame* pFrom)
{
	if(!pFrom)return;

#ifdef USE_CUDA
	cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2GRAY);//,0, m_cudaStream);
	//	m_cudaStream.waitForCompletion();
#elif USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_BGR2GRAY);
#endif

}

void CamFrame::getHSVOf(CamFrame* pFrom)
{
	if(!pFrom)return;

#ifdef USE_CUDA
	//RGB or BGR depends on device
	cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2HSV);
#elif USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_BGR2HSV);
#endif

}

void CamFrame::getBGRAOf(CamFrame* pFrom)
{
	if(!pFrom)return;

#ifdef USE_CUDA
	cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_BGR2BGRA);
#elif USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_BGR2BGRA);
#endif

}

void CamFrame::get8UC3Of(CamFrame* pFrom)
{
	if(!pFrom)return;

#ifdef USE_CUDA
	if(pFrom->getGMat()->type()==CV_8UC3)
	{
		pFrom->getGMat()->copyTo(m_GMat.m_mat);
	}
	else
	{
		cuda::cvtColor(*pFrom->getGMat(), m_GMat.m_mat, CV_GRAY2BGR);
	}
#elif USE_OPENCL

#else
	cv::cvtColor(*pFrom->getNextCMat(), m_CMat.m_mat, CV_GRAY2BGR);
#endif

}

uint64_t CamFrame::getFrameID(void)
{
	uint64_t frameID = m_CMat.m_frameID;

#ifdef USE_CUDA
	if(frameID < m_GMat.m_frameID)frameID = m_GMat.m_frameID;
#elif USE_OPENCL

#endif

	return frameID;
}

bool CamFrame::empty(void)
{
	bool bEmpty = true;
	if(!m_CMat.m_mat.empty())bEmpty = false;

#ifdef USE_CUDA
	if(!m_GMat.m_mat.empty())bEmpty = false;
#elif USE_OPENCL

#endif

	return bEmpty;
}

bool CamFrame::isNewerThan(CamFrame* pFrame)
{
	if (pFrame == NULL)return false;
	if(pFrame->getFrameID() < this->getFrameID())// m_GMat.m_frameID)
	{
		return true;
	}

	return false;
}

void CamFrame::update(CamFrame* pFrame)
{
	if (pFrame == NULL)return;

#ifdef USE_CUDA
	pFrame->getGMat()->copyTo(m_GMat.m_mat);
	updatedGMat();
#elif USE_OPENCL

#else
	pFrame->getCMat()->copyTo(m_CMat.m_mat);
	updatedCMat();
#endif
}

#ifdef USE_CUDA
void CamFrame::update(GpuMat* pGpuFrame)
{
	if (pGpuFrame == NULL)return;

	pGpuFrame->copyTo(m_GMat.m_mat);
	updatedGMat();
}
#endif

void CamFrame::update(Mat* pFrame)
{
	if (pFrame == NULL)return;

#ifdef USE_CUDA
	m_GMat.m_mat.upload(*pFrame);
	updatedGMat();
#elif USE_OPENCL

#else
	pFrame->copyTo(m_CMat.m_mat);
	updatedCMat();
#endif
}

void CamFrame::updatedCMat(void)
{
	m_CMat.m_frameID = get_time_usec();
}

#ifdef USE_CUDA
void CamFrame::updatedGMat(void)
{
	m_GMat.m_frameID = get_time_usec();
}
#endif

#ifdef USE_CUDA
GpuMat* CamFrame::getGMat(void)
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

Mat* CamFrame::getCMat(void)
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

Size CamFrame::getSize(void)
{
	Size mySize = Size(0,0);
	mySize = m_CMat.m_mat.size();

#ifdef USE_CUDA
	if(m_GMat.m_frameID > m_CMat.m_frameID)
	{
		mySize = m_GMat.m_mat.size();
	}
#elif USE_OPENCL

#endif

	return mySize;
}








}

