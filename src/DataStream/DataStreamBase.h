/*
 * VisionBase.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__DataStreamBase__H_
#define OpenKAI_src__DataStream__DataStreamBase__H_

#include "../Base/BASE.h"

namespace kai
{
	class DataStreamBase : public BASE
	{
	public:
		DataStreamBase();
		virtual ~DataStreamBase();

		virtual uint64_t tStamp(void);

	protected:
		uint64_t m_tStamp;

	};

}
#endif
