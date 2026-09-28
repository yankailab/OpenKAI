/*
 * DataStreamBase.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "DataStreamBase.h"

namespace kai
{

	DataStreamBase::DataStreamBase()
	{
	}

	DataStreamBase::~DataStreamBase()
	{
	}

	void DataStreamBase::updateTstamp(uint64_t tStamp)
	{
		if(tStamp == 0)
			m_tStamp = getTns();
		else
			m_tStamp = tStamp;
	}

	uint64_t DataStreamBase::getTstamp(void)
	{
		return m_tStamp;
	}

}
