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
	m_frameID = 0;

	m_iFrame = 0;
	m_pPrev = &m_pFrame[m_iFrame];
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];
}

CamFrame::~CamFrame()
{
}

void CamFrame::getResizedOf(CamFrame* pFrom, int width, int height)
{
	if(!pFrom)return;
	if(pFrom->getCurrentFrame()->empty())return;

	cv::Size newSize = cv::Size(width,height);

	if(newSize == pFrom->getCurrentFrame()->size())
	{
		this->updateFrame(pFrom);
	}
	else
	{
		cuda::resize(*pFrom->getCurrentFrame(), m_GMat, newSize);
		this->updateFrame(&m_GMat);
	}

}

void CamFrame::getGrayOf(CamFrame* pFrom)
{
	if(!pFrom)return;

	cuda::cvtColor(*pFrom->getCurrentFrame(), *m_pNext, CV_BGR2GRAY);//,0, m_cudaStream);

//	m_cudaStream.waitForCompletion();
}

void CamFrame::getHSVOf(CamFrame* pFrom)
{
	if(!pFrom)return;

	//RGB or BGR depends on device
	cuda::cvtColor(*pFrom->getCurrentFrame(), *m_pNext, CV_BGR2HSV);
}

void CamFrame::getBGRAOf(CamFrame* pFrom)
{
	if(!pFrom)return;

	cuda::cvtColor(*pFrom->getCurrentFrame(), *m_pNext, CV_BGR2BGRA);
}

void CamFrame::get8UC3Of(CamFrame* pFrom)
{
	if(!pFrom)return;

	if(pFrom->getCurrentFrame()->type()==CV_8UC3)
	{
		pFrom->getCurrentFrame()->copyTo(*m_pNext);
	}
	else
	{
		cuda::cvtColor(*pFrom->getCurrentFrame(), *m_pNext, CV_GRAY2BGR);
	}
}

GpuMat* CamFrame::getCurrentFrame(void)
{
	return m_pNext;
}

GpuMat* CamFrame::getPreviousFrame(void)
{
	return m_pPrev;
}

uint64_t CamFrame::getFrameID(void)
{
	return m_frameID;
}

bool CamFrame::empty(void)
{
	return m_pNext->empty();
}

bool CamFrame::isNewerThan(CamFrame* pFrame)
{
	if (pFrame == NULL)return false;
	if(pFrame->getFrameID() < m_frameID)
	{
		return true;
	}

	return false;
}

void CamFrame::switchFrame(void)
{
	//switch the current frame and old frame
	m_pPrev = m_pNext;
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];
}

void CamFrame::updateFrame(CamFrame* pFrame)
{
	if (pFrame == NULL)return;

	pFrame->getCurrentFrame()->copyTo(*m_pNext);
	m_frameID = get_time_usec();
}


void CamFrame::updateFrame(GpuMat* pGpuFrame)
{
	if (pGpuFrame == NULL)return;

	pGpuFrame->copyTo(*m_pNext);
	m_frameID = get_time_usec();
}

void CamFrame::updateFrame(Mat* pFrame)
{
	if (pFrame == NULL)return;

	m_pNext->upload(*pFrame);
	m_frameID = get_time_usec();
}


void CamFrame::updateFrameSwitch(CamFrame* pFrame)
{
	if (pFrame == NULL)return;

	//Prepare the destination
	GpuMat* pDest = m_pPrev;

	//Pointer both frames to the one not being transfered temporarily
	m_pPrev = m_pNext;

	pFrame->getCurrentFrame()->copyTo(*pDest);

	//Update the pointer to the latest frame
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];

	m_frameID = get_time_usec();
}


void CamFrame::updateFrameSwitch(GpuMat* pGpuFrame)
{
	if (pGpuFrame == NULL)return;

	//Prepare the destination
	GpuMat* pDest = m_pPrev;

	//Pointer both frames to the one not being transfered temporarily
	m_pPrev = m_pNext;

	pGpuFrame->copyTo(*pDest);

	//Update the pointer to the latest frame
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];

	m_frameID = get_time_usec();
}

void CamFrame::updateFrameSwitch(Mat* pFrame)
{
	if (pFrame == NULL)return;

	//Prepare the destination
	GpuMat* pDest = m_pPrev;

	//Pointer both frames to the one not being transfered temporarily
	m_pPrev = m_pNext;

	pDest->upload(*pFrame);

	//Update the pointer to the latest frame
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];

	m_frameID = get_time_usec();

}


}

