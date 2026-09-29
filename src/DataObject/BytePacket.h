/*
 * BytePacket.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__BytePacket__H_
#define OpenKAI_src__DataStream__BytePacket__H_

#include "DataObjBase.h"

namespace kai
{
	class BytePacket : public DataObjBase
	{
	public:
		BytePacket();
		virtual ~BytePacket();
		void console(void *pConsole) override;


	protected:
	};

}
#endif
