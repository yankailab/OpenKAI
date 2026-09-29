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

	void RGBDframe::set(const Mat &rgb, const Mat &depth, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);
		rgb.copyTo(m_mRGB);
		depth.copyTo(m_mD);
		updateTstamp(tStamp);
	}

	uint64_t RGBDframe::get(Mat &rgb, Mat &depth)
	{
		std::shared_lock lock(m_sMutex);
		m_mRGB.copyTo(rgb);
		m_mD.copyTo(depth);
		return getTstamp();
	}

	void RGBDframe::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
