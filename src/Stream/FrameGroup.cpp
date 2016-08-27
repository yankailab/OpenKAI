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
	m_iFrame = 0;
	m_numFrame = 0;
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
		m_pFrame[m_iFrame] = new Frame();
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

Frame* FrameGroup::getPrevFrame(void)
{
	//Thread safe
	int iFrame = m_iFrame;
	if(iFrame==0)
	{
		return m_pFrame[m_numFrame-1];
	}

	return m_pFrame[iFrame-1];
}

Frame* FrameGroup::getLastFrame(void)
{
	return m_pFrame[m_iFrame];
}

}

