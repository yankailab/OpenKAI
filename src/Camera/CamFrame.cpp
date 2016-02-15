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
	m_frameID_GPU = 0;

	m_iFrame_GPU = 0;
	m_pPrev = &m_pFrame[m_iFrame_GPU];
	m_iFrame_GPU = 1 - m_iFrame_GPU;
	m_pNext = &m_pFrame[m_iFrame_GPU];
}

CamFrame::~CamFrame()
{
}

void CamFrame::getResizedOf(CamFrame* pFrom, int width, int height)
{
	if(!pFrom)return;
	if(pFrom->getCurrent()->empty())return;

	cv::Size newSize = cv::Size(width,height);

	if(newSize == pFrom->getCurrent()->size())
	{
		this->update(pFrom);
	}
	else
	{
		cuda::resize(*pFrom->getCurrent(), *m_pNext, newSize);
//		cuda::resize(*pFrom->getCurrentFrame(), m_GMat, newSize);
//		this->updateFrame(&m_GMat);

		m_frameID_GPU = get_time_usec();
	}

}

void CamFrame::getGrayOf(CamFrame* pFrom)
{
	if(!pFrom)return;

#ifdef USE_CUDA
	cuda::cvtColor(*pFrom->getCurrent(), *m_pNext, CV_BGR2GRAY);//,0, m_cudaStream);
#else
	cv::cvtColor(pFrom->get);
#endif

//	m_cudaStream.waitForCompletion();
}

void CamFrame::getHSVOf(CamFrame* pFrom)
{
	if(!pFrom)return;

	//RGB or BGR depends on device
	cuda::cvtColor(*pFrom->getCurrent(), *m_pNext, CV_BGR2HSV);
}

void CamFrame::getBGRAOf(CamFrame* pFrom)
{
	if(!pFrom)return;

	cuda::cvtColor(*pFrom->getCurrent(), *m_pNext, CV_BGR2BGRA);
}

void CamFrame::get8UC3Of(CamFrame* pFrom)
{
	if(!pFrom)return;

	if(pFrom->getCurrent()->type()==CV_8UC3)
	{
		pFrom->getCurrent()->copyTo(*m_pNext);
	}
	else
	{
		cuda::cvtColor(*pFrom->getCurrent(), *m_pNext, CV_GRAY2BGR);
	}
}

GpuMat* CamFrame::getCurrent(void)
{
	return m_pNext;
}

GpuMat* CamFrame::getPrevious(void)
{
	return m_pPrev;
}

uint64_t CamFrame::getFrameID(void)
{
	return m_frameID_GPU;
}

bool CamFrame::empty(void)
{
	return m_pNext->empty();
}

bool CamFrame::isNewerThan(CamFrame* pFrame)
{
	if (pFrame == NULL)return false;
	if(pFrame->getFrameID() < m_frameID_GPU)
	{
		return true;
	}

	return false;
}

void CamFrame::switchFrame(void)
{
	//switch the current frame and old frame
	m_pPrev = m_pNext;
	m_iFrame_GPU = 1 - m_iFrame_GPU;
	m_pNext = &m_pFrame[m_iFrame_GPU];
}

void CamFrame::update(CamFrame* pFrame)
{
	if (pFrame == NULL)return;

	pFrame->getCurrent()->copyTo(*m_pNext);
	m_frameID_GPU = get_time_usec();
}


void CamFrame::update(GpuMat* pGpuFrame)
{
	if (pGpuFrame == NULL)return;

	pGpuFrame->copyTo(*m_pNext);
	m_frameID_GPU = get_time_usec();
}

void CamFrame::update(Mat* pFrame)
{
	if (pFrame == NULL)return;

	m_pNext->upload(*pFrame);
	m_frameID_GPU = get_time_usec();
}


void CamFrame::updateSwitch(CamFrame* pFrame)
{
	if (pFrame == NULL)return;

	//Prepare the destination
	GpuMat* pDest = m_pPrev;

	//Pointer both frames to the one not being transfered temporarily
	m_pPrev = m_pNext;

	pFrame->getCurrent()->copyTo(*pDest);

	//Update the pointer to the latest frame
	m_iFrame_GPU = 1 - m_iFrame_GPU;
	m_pNext = &m_pFrame[m_iFrame_GPU];

	m_frameID_GPU = get_time_usec();
}


void CamFrame::updateSwitch(GpuMat* pGpuFrame)
{
	if (pGpuFrame == NULL)return;

	//Prepare the destination
	GpuMat* pDest = m_pPrev;

	//Pointer both frames to the one not being transfered temporarily
	m_pPrev = m_pNext;

	pGpuFrame->copyTo(*pDest);

	//Update the pointer to the latest frame
	m_iFrame_GPU = 1 - m_iFrame_GPU;
	m_pNext = &m_pFrame[m_iFrame_GPU];

	m_frameID_GPU = get_time_usec();
}

void CamFrame::updateSwitch(Mat* pFrame)
{
	if (pFrame == NULL)return;

	//Prepare the destination
	GpuMat* pDest = m_pPrev;

	//Pointer both frames to the one not being transfered temporarily
	m_pPrev = m_pNext;

	pDest->upload(*pFrame);

	//Update the pointer to the latest frame
	m_iFrame_GPU = 1 - m_iFrame_GPU;
	m_pNext = &m_pFrame[m_iFrame_GPU];

	m_frameID_GPU = get_time_usec();

}


}

