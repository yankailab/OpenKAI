/*
 * DataObjects.h
 *
 *  Created on: Nov 22, 2016
 *      Author: root
 */

#ifndef OpenKAI_src_Instance_DataStreams_H_
#define OpenKAI_src_Instance_DataStreams_H_

// Data streams
#include "../DataObject/BBoxStream.h"
#include "../DataObject/BytePacketStream.h"
#include "../DataObject/IMUstream.h"
#include "../DataObject/LineFrame.h"
#include "../DataObject/MavlinkStream.h"
#include "../DataObject/PCLframe.h"
#include "../DataObject/PCLmap.h"
#include "../DataObject/SharedMemoryFrame.h"
#include "../DataObject/UGLIDcellStream.h"
#ifdef USE_OPENCV
#include "../DataObject/RGBframe.h"
#include "../DataObject/RGBDframe.h"
#endif


#define ADD_DATA_STREAM(x)     \
	if (name == #x)             \
	{                           \
		return createInst<x>(); \
	}

namespace kai
{

	class DataObjects
	{
	public:
		DataObjects();
		virtual ~DataObjects();
		DataObjBase *createInstance(const string &name);

	private:
		template <typename T>
		DataObjBase *createInst(void);
	};

}

#endif
