/*
 * BytePacket.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "BytePacket.h"

namespace kai
{

	BytePacket::BytePacket()
	{
	}

	BytePacket::~BytePacket()
	{
	}

	void BytePacket::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
