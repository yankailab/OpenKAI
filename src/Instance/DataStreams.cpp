/*
 * DataStreams.cpp
 *
 *  Created on: Nov 22, 2016
 *      Author: yankai
 */

#include "DataStreams.h"

namespace kai
{

	DataStreams::DataStreams()
	{
	}

	DataStreams::~DataStreams()
	{
	}

	DataStreamBase *DataStreams::createInstance(const string &name)
	{
		IF_N(name.empty());

		ADD_DATA_STREAM(BytePacket);
		ADD_DATA_STREAM(RGBframe);
		ADD_DATA_STREAM(RGBDframe);

		return nullptr;
	}

	template <typename T>
	DataStreamBase *DataStreams::createInst(void)
	{
		T *pInst = new T();
		return pInst;
	}

}
