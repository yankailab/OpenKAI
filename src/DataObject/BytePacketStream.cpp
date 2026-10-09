/*
 * BytePacketStream.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "BytePacketStream.h"
#include <limits>

namespace kai
{

	BytePacketStream::BytePacketStream()
	{
		clear();
	}

	BytePacketStream::~BytePacketStream()
	{
	}

	bool BytePacketStream::loadConfig(void)
	{
		IF_F(!this->DataObjStream::loadConfig());
		json &j = *m_pJ;

		jKv(j, "nPacket", m_nPacket);
		jKv(j, "nPbuf", m_nPbuf);
		IF_Le_F(m_nPacket <= 0, "Invalid BytePacketStream capacity");

		return clear(m_nPacket, m_nPbuf);
	}

	bool BytePacketStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjStream::saveConfig(false));
		{
			std::shared_lock lock(m_sMutex);
			json &j = *m_pJ;
			j["nPacket"] = m_nPacket;
			j["nPbuf"] = m_nPbuf;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool BytePacketStream::clear(size_t nPacket, size_t nPacketBuf)
	{
		std::unique_lock lock(m_sMutex);

		IF_F((nPacket > std::numeric_limits<int>::max()) || (nPacketBuf > std::numeric_limits<int>::max()));

		if (nPacket > 0)
		{
			m_nPacket = nPacket;
		}

		if (nPacketBuf > 0)
		{
			m_nPbuf = nPacketBuf;
		}

		IF_F(m_nPacket <= 0);

		m_vElement.resize(m_nPacket);
		for (BYTE_PACKET &bp : m_vElement)
		{
			bp.clear(m_nPbuf);
		}

		m_iBset = 0;
		// Keep the timestamp watermark so existing readers survive a clear.
		updateTstamp();

		return true;
	}

	void BytePacketStream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjStream::console(pConsole);
	}

}
