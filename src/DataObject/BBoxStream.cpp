/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "BBoxStream.h"

namespace kai
{

	BBoxStream::BBoxStream()
	{
	}

	BBoxStream::~BBoxStream()
	{
	}

	void BBoxStream::set(const vector<BBOX_OBJ>& vSrc, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);
		m_vObj = vSrc;
		updateTstamp(tStamp);
	}

	uint64_t BBoxStream::get(vector<BBOX_OBJ>& vDest)
	{
		std::shared_lock lock(m_sMutex);
		vDest = m_vObj;
		return getTstamp();
	}

}
