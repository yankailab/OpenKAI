/*
 * PCLframe.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__PCLframe__H_
#define OpenKAI_src__DataStream__PCLframe__H_

#include "PCLstream.h"

namespace kai
{
	class PCLframe : public PCLstream
	{
	public:
		PCLframe();
		virtual ~PCLframe();
		void console(void *pConsole) override;

		// Replace the whole frame and match capacity to its size; use set or add exclusively.
		void set(const vector<GEOMETRY_POINT> &src, uint64_t tStamp = 0);
		// Copy all slots in storage order, including unused/invalid points in a stream buffer.
		uint64_t get(vector<GEOMETRY_POINT> &dest);
		using PCLstream::get;
	};

}
#endif
