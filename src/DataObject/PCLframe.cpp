/*
 * PCLframe.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "PCLframe.h"

namespace kai
{
	void PCLframe::set(const vector<GEOMETRY_POINT> &src, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);
		m_nBuf = src.size();
		m_vElement = src;
		m_iBset = 0;
		updateTstamp(tStamp);
	}

	uint64_t PCLframe::get(vector<GEOMETRY_POINT> &dest)
	{
		std::shared_lock lock(m_sMutex);
		dest = m_vElement;
		return getTstamp();
	}
}
