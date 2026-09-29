/*
 * DataObjBase.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "DataObjBase.h"

namespace kai
{

	DataObjBase::DataObjBase()
	{
	}

	DataObjBase::~DataObjBase()
	{
	}

	void DataObjBase::updateTstamp(uint64_t tStamp)
	{
		m_tStamp.store(tStamp ? tStamp : getTns());
	}

	uint64_t DataObjBase::getTstamp(void)
	{
		return m_tStamp.load();
	}

}
