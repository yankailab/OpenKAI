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
	m_width = 0;
	m_height = 0;

	m_centerH = 0;
	m_centerV = 0;

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
	// Start mutex
/*	if (pthread_mutex_init(&m_mutexNext, NULL) != 0)
	{
		printf("mutexNext init failed\n");
	}
*/
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

void CamFrame::getBGRA(CamFrame* pResult)
{
	if(!pResult)return;

#ifdef USE_CUDA
	cuda::cvtColor(*m_pNext, *(pResult->m_pNext), CV_BGR2BGRA);
#else


#endif
}


void CamFrame::rotate(double radian)
{
    Point2f center(m_centerH, m_centerV);
    double deg = radian * 180.0 * OneOvPI;
    Mat rot = getRotationMatrix2D(center, deg, 1.0);

#ifdef USE_CUDA
//    pthread_mutex_lock(&m_mutexNext);
	cuda::warpAffine(*m_pNext, m_GMat, rot, m_pNext->size());
//    pthread_mutex_unlock(&m_mutexNext);
#else
	//cv::warpAffine(m_uFrame, m_uFrame, rot, m_uFrame.size());

#endif
}

void CamFrame::getNextMat(Mat* pDest)
{
//    pthread_mutex_lock(&m_mutexNext);
    m_GMat.download(*pDest);
//    pthread_mutex_unlock(&m_mutexNext);
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
	m_width = m_uFrame.cols;
	m_height = m_uFrame.rows;
	m_centerH = m_width * 0.5;
	m_centerV = m_height * 0.5;

	//Upload to GPU
#ifdef USE_CUDA
	m_pNext->upload(m_uFrame);
#else


#endif


}





}

