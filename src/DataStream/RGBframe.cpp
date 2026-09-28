/*
 * RGBframe.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "RGBframe.h"

namespace kai
{

	RGBframe::RGBframe()
	{
	}

	RGBframe::~RGBframe()
	{
	}

	void RGBframe::copyFrom(const Mat &src)
	{
		Mat m = src.clone();

		{
			std::unique_lock lock(m_sMutex);
			swap(m, m_mRGB);
		}
	}

	void RGBframe::copyTo(Mat &dest)
	{
		Mat m;
		{
			std::shared_lock lock(m_sMutex);
			m = m_mRGB;
		}

		m.copyTo(dest);
	}

	void RGBframe::console(void *pConsole)
	{
		NULL_(pConsole);
		DataStreamBase::console(pConsole);
	}

}
