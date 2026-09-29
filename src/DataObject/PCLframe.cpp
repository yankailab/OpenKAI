/*
 * PCLframe.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "PCLframe.h"

namespace kai
{
	PCLframe::PCLframe()
	{
	}

	PCLframe::~PCLframe()
	{
	}

	void PCLframe::set(const vector<GEOMETRY_POINT> &src, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);
		m_vPoints = src;
		updateTstamp(tStamp);
	}

	uint64_t PCLframe::get(vector<GEOMETRY_POINT> &dest)
	{
		std::shared_lock lock(m_sMutex);
		dest = m_vPoints;
		return getTstamp();
	}

	void PCLframe::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
