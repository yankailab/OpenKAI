/*
 * DataObjBase.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__DataStreamBase__H_
#define OpenKAI_src__DataStream__DataStreamBase__H_

#include "../Base/BASE.h"
#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace kai
{
	class DataObjBase : public BASE
	{
	public:
		DataObjBase();
		virtual ~DataObjBase();

		void updateTstamp(uint64_t tStamp = 0);
		uint64_t getTstamp(void);

	protected:
		// m_tStamp the last time this data object is updated, not for each of its element.
		std::atomic<uint64_t> m_tStamp{0};

	};

}
#endif
