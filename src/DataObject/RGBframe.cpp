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

	void RGBframe::set(const Mat &src, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);
		src.copyTo(m_mRGB);
		updateTstamp(tStamp);
	}

	uint64_t RGBframe::get(Mat &dest)
	{
		std::shared_lock lock(m_sMutex);
		m_mRGB.copyTo(dest);
		return getTstamp();
	}

	void RGBframe::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
