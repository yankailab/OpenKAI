/*
 * DataStreams.h
 *
 *  Created on: Nov 22, 2016
 *      Author: root
 */

#ifndef OpenKAI_src_Instance_DataStreams_H_
#define OpenKAI_src_Instance_DataStreams_H_

// Data streams
#include "../DataStream/BytePacket.h"
#include "../DataStream/PCLframe.h"
#include "../DataStream/PCLmap.h"
#include "../DataStream/LineFrame.h"
#include "../DataStream/IMUstream.h"
#ifdef USE_OPENCV
#include "../DataStream/RGBframe.h"
#include "../DataStream/RGBDframe.h"
#endif


#define ADD_DATA_STREAM(x)     \
	if (name == #x)             \
	{                           \
		return createInst<x>(); \
	}

namespace kai
{

	class DataStreams
	{
	public:
		DataStreams();
		virtual ~DataStreams();
		DataStreamBase *createInstance(const string &name);

	private:
		template <typename T>
		DataStreamBase *createInst(void);
	};

}

#endif
