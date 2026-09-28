/*
 * BytePacket.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__BytePacket__H_
#define OpenKAI_src__DataStream__BytePacket__H_

#include "DataStreamBase.h"

namespace kai
{
	class BytePacket : public DataStreamBase
	{
	public:
		BytePacket();
		virtual ~BytePacket();
		void console(void *pConsole) override;


	protected:
	};

}
#endif
