/*
 * DataObjects.cpp
 *
 *  Created on: Nov 22, 2016
 *      Author: yankai
 */

#include "DataObjects.h"

namespace kai
{

	DataObjects::DataObjects()
	{
	}

	DataObjects::~DataObjects()
	{
	}

	DataObjBase *DataObjects::createInstance(const string &name)
	{
		IF_N(name.empty());

		ADD_DATA_STREAM(BBoxStream);
		ADD_DATA_STREAM(BytePacket);
		ADD_DATA_STREAM(IMUstream);
		ADD_DATA_STREAM(LineFrame);
		ADD_DATA_STREAM(PCLframe);
		ADD_DATA_STREAM(PCLmap);
		ADD_DATA_STREAM(UGLIDcellStream);
#ifdef USE_OPENCV
		ADD_DATA_STREAM(RGBframe);
		ADD_DATA_STREAM(RGBDframe);
#endif

		return nullptr;
	}

	template <typename T>
	DataObjBase *DataObjects::createInst(void)
	{
		T *pInst = new T();
		return pInst;
	}

}
