/*
 * LineFrame.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "LineFrame.h"

namespace kai
{
	LineFrame::LineFrame()
	{
	}

	LineFrame::~LineFrame()
	{
	}

	void LineFrame::set(const vector<GEOMETRY_LINE> &src, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);
		m_vLines = src;
		updateTstamp(tStamp);
	}

	uint64_t LineFrame::get(vector<GEOMETRY_LINE> &dest)
	{
		std::shared_lock lock(m_sMutex);
		dest = m_vLines;
		return getTstamp();
	}

	void LineFrame::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
