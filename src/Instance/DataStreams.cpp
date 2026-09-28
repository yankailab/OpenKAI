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
		ADD_DATA_STREAM(PCLframe);
		ADD_DATA_STREAM(PCLmap);
		ADD_DATA_STREAM(LineFrame);
		ADD_DATA_STREAM(IMUstream);
#ifdef USE_OPENCV
		ADD_DATA_STREAM(RGBframe);
		ADD_DATA_STREAM(RGBDframe);
#endif

		return nullptr;
	}

	template <typename T>
	DataStreamBase *DataStreams::createInst(void)
	{
		T *pInst = new T();
		return pInst;
	}

}
