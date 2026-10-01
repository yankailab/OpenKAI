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

	bool BytePacket::loadConfig(void)
	{
		IF_F(!this->DataObjBase::loadConfig());
		json &j = *m_pJ;

		jKv(j, "nPacket", m_nPacket);
		jKv(j, "nPbuf", m_nPbuf);

		return clear(m_nPacket, m_nPbuf);
	}

	bool BytePacket::saveConfig(bool bExport)
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

	bool BytePacket::clear(size_t nPacket, size_t nPacketBuf)
	{
		std::unique_lock lock(m_sMutex);

		m_vPacket.clear();
		if (nPacket > 0)
		{
			m_vPacket.reserve(nPacket);
		}

		for (BYTE_PACKET &bp : m_vPacket)
		{
			bp.clear(nPacketBuf);
		}

		m_iPset = 0;

		return true;
	}

	void BytePacket::addPacket(const vector<uint8_t> &vB, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);

		BYTE_PACKET *pBp = &m_vPacket[m_iPset];
		pBp->set(vB, tStamp);

		if (m_iPset == m_vPacket.size() - 1)
		{
			m_iPset = 0;
		}
		else
		{
			m_iPset++;
		}
	}

	void BytePacket::getPackets(vector<uint8_t> &vB, uint64_t tStampFrom)
	{
		std::shared_lock lock(m_sMutex);
		// TODO: put packets from m_vPacket whose tStamp > tStampFrom into vB
	}

	void BytePacket::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
