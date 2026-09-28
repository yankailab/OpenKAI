/*
 * DataStreamBase.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__DataStreamBase__H_
#define OpenKAI_src__DataStream__DataStreamBase__H_

#include "../Base/BASE.h"
#include <mutex>
#include <shared_mutex>

namespace kai
{
	class DataStreamBase : public BASE
	{
	public:
		DataStreamBase();
		virtual ~DataStreamBase();

	};

}
#endif
