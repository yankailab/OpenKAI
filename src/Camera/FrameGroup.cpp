/*
 * CameraInput.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "FrameGroup.h"

namespace kai
{

FrameGroup::FrameGroup()
{
}

FrameGroup::~FrameGroup()
{
	int i;
	for(i=0; i<m_numFrame; i++)
	{
		delete m_pFrame[i];
	}
}

bool FrameGroup::init(int numFrame)
{
	if(numFrame > NUM_FRAME)return false;

	m_numFrame = numFrame;

	for(m_iFrame=0; m_iFrame<m_numFrame; m_iFrame++)
	{
		m_pFrame[m_iFrame] = new CamFrame();
	}

	m_iFrame = 0;
	return true;
}

void FrameGroup::updateFrameIndex(void)
{
	//Thread safe
	int iFrame = m_iFrame+1;
	if(iFrame >= m_numFrame)iFrame = 0;

	m_iFrame = iFrame;
}

CamFrame* FrameGroup::getPrevFrame(void)
{
	//Thread safe
	int iFrame = m_iFrame;
	if(iFrame==0)
	{
		return m_pFrame[m_numFrame-1];
	}

	return m_pFrame[iFrame-1];
}

CamFrame* FrameGroup::getLastFrame(void)
{
	return m_pFrame[m_iFrame];
}

//void FrameGroup::updateSwitch(FrameGroup* pFrame)
//{
//	if (pFrame == NULL)return;
//
//	//Prepare the destination
//	GpuMat* pDest = &m_pGPrev->m_mat;
//
//	//Pointer both frames to the one not being transfered temporarily
//	m_pGPrev = m_pGNext;
//
//	pFrame->getGMat()->copyTo(*pDest);
//
//	//Update the pointer to the latest frame
//	m_iGFrame = 1 - m_iGFrame;
//	m_pGNext = &m_pGFrame[m_iGFrame];
//
//	updateFrameID(&m_GMat.m_frameID);
//}
//
//
//void FrameGroup::updateSwitch(GpuMat* pGpuFrame)
//{
//	if (pGpuFrame == NULL)return;
//
//	//Prepare the destination
//	GpuMat* pDest = &m_pGPrev->m_mat;
//
//	//Pointer both frames to the one not being transfered temporarily
//	m_pGPrev = m_pGNext;
//
//	pGpuFrame->copyTo(*pDest);
//
//	//Update the pointer to the latest frame
//	m_iGFrame = 1 - m_iGFrame;
//	m_pGNext = &m_pGFrame[m_iGFrame];
//
//	updateFrameID(&m_GMat.m_frameID);
//}
//
//void FrameGroup::updateSwitch(Mat* pFrame)
//{
//	if (pFrame == NULL)return;
//
//	//Prepare the destination
//	GpuMat* pDest = &m_pGPrev->m_mat;
//
//	//Pointer both frames to the one not being transfered temporarily
//	m_pGPrev = m_pGNext;
//
//	pDest->upload(*pFrame);
//
//	//Update the pointer to the latest frame
//	m_iGFrame = 1 - m_iGFrame;
//	m_pGNext = &m_pGFrame[m_iGFrame];
//
//	updateFrameID(&m_GMat.m_frameID);
//
//}


}

