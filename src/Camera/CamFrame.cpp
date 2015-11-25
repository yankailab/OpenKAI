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

bool CamFrame::init(void)
{
	return true;
}

void CamFrame::getResized(int width, int height, CamFrame* pResult)
{
	if(!pResult)return;
	cv::Size newSize = cv::Size(width,height);

	if(newSize == m_pNext->size())
	{
		pResult->updateFrame(m_pNext);
	}
	else
	{
		cuda::resize(*m_pNext,m_GMat,newSize);
		pResult->updateFrame(&m_GMat);
	}

}

void CamFrame::getGray(CamFrame* pResult)
{
	if(!pResult)return;

	cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_BGR2GRAY);
}

void CamFrame::getHSV(CamFrame* pResult)
{
	if(!pResult)return;

	//RGB or BGR depends on device
	cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_BGR2HSV);
}

void CamFrame::getBGRA(CamFrame* pResult)
{
	if(!pResult)return;

	cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_BGR2BGRA);
}

void CamFrame::get8UC3(CamFrame* pResult)
{
	if(!pResult)return;

	if(m_pNext->type()==CV_8UC3)
	{
		m_pNext->copyTo(*pResult->m_pNext);
	}
	else
	{
		cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_GRAY2BGR);
	}
}

void CamFrame::copyTo(CamFrame* pResult)
{
	if(!pResult)return;

	m_pNext->copyTo(*pResult->m_pNext);
}

void CamFrame::switchFrame(void)
{
	//switch the current frame and old frame
	m_pPrev = m_pNext;
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];
}

void CamFrame::updateFrame(GpuMat* pGpuFrame)
{
	if (pGpuFrame == NULL)return;

	//Update the frame ID for flow synchronization
	if (++m_frameID == MAX_FRAME_ID)
	{
		m_frameID = 0;
	}

	pGpuFrame->copyTo(*m_pNext);
}

void CamFrame::updateFrame(Mat* pFrame)
{
	if (pFrame == NULL)return;

	//Update the frame ID for flow synchronization
	if (++m_frameID == MAX_FRAME_ID)
	{
		m_frameID = 0;
	}

	m_pNext->upload(*pFrame);
}


GpuMat* CamFrame::getCurrentFrame(void)
{
	return m_pNext;
}




}

