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
		IF_F(!this->DataObjBase::loadConfig());
		json &j = *m_pJ;

		jKv(j, "nPacket", m_nPacket);
		jKv(j, "nPbuf", m_nPbuf);
		IF_Le_F(m_nPacket <= 0 || m_nPbuf < 0, "Invalid BytePacketStream capacity");

		return clear(m_nPacket, m_nPbuf);
	}

	bool BytePacketStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjBase::saveConfig(false));
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

		IF_F(m_nPacket <= 0 || m_nPbuf < 0);

		m_vPacket.resize(m_nPacket);
		for (BYTE_PACKET &bp : m_vPacket)
		{
			bp.clear(m_nPbuf);
		}

		m_iPset = 0;
		// Keep the timestamp watermark so existing readers survive a clear.
		updateTstamp();

		return true;
	}

	void BytePacketStream::addPacket(const vector<uint8_t> &vB, uint64_t tStamp)
	{
		IF_(vB.empty());

		std::unique_lock lock(m_sMutex);
		if (tStamp == 0)
		{
			tStamp = getTns();
		}

		// Timestamp cursors must distinguish every arrival, including equal or
		// out-of-order producer timestamps and writes by multiple producers.
		tStamp = std::max(tStamp, m_tLastPacket + 1);
		m_vPacket[m_iPset].set(vB, tStamp);
		m_tLastPacket = tStamp;

		m_iPset = (m_iPset + 1) % m_vPacket.size();
		updateTstamp(tStamp);
	}

	void BytePacketStream::getPackets(vector<BYTE_PACKET> &vBp, uint64_t tStampFrom)
	{
		std::shared_lock lock(m_sMutex);

		vBp.clear();
		const size_t nPacket = m_vPacket.size();
		vBp.reserve(nPacket);

		for (size_t n = 0; n < nPacket; ++n)
		{
			const size_t iPacket = (m_iPset + n) % nPacket;
			const BYTE_PACKET &bp = m_vPacket[iPacket];

			IF_CONT(bp.m_tStamp <= tStampFrom);

			vBp.push_back(bp);
		}
	}

	void BytePacketStream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
