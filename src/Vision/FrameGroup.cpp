/*
 * CameraInput.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "../Vision/FrameGroup.h"

namespace kai
{

	FrameGroup::FrameGroup()
	{
		m_iFrame = 0;
		m_nFrame = 0;
	}

	FrameGroup::~FrameGroup()
	{
	}

	bool FrameGroup::init(int nFrame)
	{
		if (nFrame > FRAMGROUP_N_FRAME)
			return false;

		m_nFrame = nFrame;
		m_iFrame = 0;

		return true;
	}

	void FrameGroup::updateFrameIndex(void)
	{
		int iFrame = m_iFrame + 1;
		if (iFrame >= m_nFrame)
			iFrame = 0;

		m_iFrame = iFrame;
	}

	Frame *FrameGroup::getPrevFrame(void)
	{
		if (m_iFrame == 0)
		{
			return &m_pFrame[m_nFrame - 1];
		}

		return &m_pFrame[m_iFrame - 1];
	}

	Frame *FrameGroup::getLastFrame(void)
	{
		return &m_pFrame[m_iFrame];
	}

}
