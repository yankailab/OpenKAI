/*
 * RGBDframe.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "RGBDframe.h"

namespace kai
{

	RGBDframe::RGBDframe()
	{
	}

	RGBDframe::~RGBDframe()
	{
	}

	void RGBDframe::copyFrom(const Mat &srcRGB, const Mat &srcD)
	{
		Mat mRGB = srcRGB.clone();
		Mat mD = srcD.clone();

		{
			std::unique_lock lock(m_sMutex);
			swap(mRGB, m_mRGB);
			swap(mD, m_mD);
		}
	}

	void RGBDframe::copyTo(Mat &destRGB, Mat &destD)
	{
		Mat mRGB;
		Mat mD;
		{
			std::shared_lock lock(m_sMutex);
			mRGB = m_mRGB;
			mD = m_mD;
		}

		mRGB.copyTo(destRGB);
		mD.copyTo(destD);
	}

	void RGBDframe::console(void *pConsole)
	{
		NULL_(pConsole);
		DataStreamBase::console(pConsole);
	}

}
