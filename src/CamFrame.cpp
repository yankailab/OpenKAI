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
	// TODO Auto-generated constructor stub

}

CamFrame::~CamFrame()
{
	// TODO Auto-generated destructor stub
}

bool CamFrame::init(void)
{
	m_frameID = 0;
	m_iFrame = 0;
	m_width = 0;
	m_height = 0;

	m_pPrev = &m_pFrame[m_iFrame];
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];

	return true;
}

void CamFrame::resize(int width, int height, CamFrame* pResult)
{
	if(!pResult)return;

	cv::Size newSize = cv::Size(width,height);

#ifdef USE_CUDA
	Mat newMat;
	cv::resize(m_uFrame, newMat, newSize);
	pResult->updateFrame(&newMat);
#else


#endif

}

void CamFrame::getGray(CamFrame* pResult)
{
	if(!pResult)return;

#ifdef USE_CUDA
	cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_BGR2GRAY);
#else


#endif
}

void CamFrame::getHSV(CamFrame* pResult)
{
	if(!pResult)return;

#ifdef USE_CUDA
	//RGB or BGR depends on device
	cuda::cvtColor(*m_pNext, *(pResult->m_pNext), CV_BGR2HSV);
#else


#endif
}

void CamFrame::switchFrame(void)
{
	//switch the current frame and old frame
	m_pPrev = m_pNext;
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];
}

void CamFrame::updateFrame(Mat* pFrame)
{
	if (pFrame == NULL)return;

	//Update the frame ID for flow synchronization
	if (++m_frameID == MAX_FRAME_ID)
	{
		m_frameID = 0;
	}

	m_uFrame = *pFrame;

	//Upload to GPU
#ifdef USE_CUDA
	m_pNext->upload(m_uFrame);
#else


#endif


}





}

