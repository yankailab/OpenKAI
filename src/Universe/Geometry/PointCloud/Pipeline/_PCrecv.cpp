/*
 * _PCrecv.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#include "_PCrecv.h"

namespace kai
{
	_PCrecv::_PCrecv()
	{
	}

	_PCrecv::~_PCrecv()
	{
	}

	bool _PCrecv::loadConfig(void)
	{
		IF_F(!_ReferenceFrame::loadConfig());
		jKv(*m_pJ, "nPmax", m_nPmax);
		IF_Le_F(m_nPmax == 0, "nPmax must be positive");
		m_vPacket.reserve(pcstream::maxPacketBytes);
		return true;
	}

	bool _PCrecv::saveConfig(bool bExport)
	{
		IF_F(!_ReferenceFrame::saveConfig(false));
		(*m_pJ)["nPmax"] = m_nPmax;
		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _PCrecv::link(InstanceMgr *pM)
	{
		IF_F(!_ReferenceFrame::link(pM));
		string name;
		jKv(*m_pJ, "PCLframeOut", name);
		m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(name)));
		IF_Le_F(!m_pPCLout, "PCLframeOut not found: " + name);

		name.clear();
		jKv(*m_pJ, "BytePacketStreamIn", name);
		m_pBpStreamIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(name)));
		IF_Le_F(!m_pBpStreamIn, "BytePacketStreamIn not found: " + name);
		m_tLastBpStreamIn = 0;
		return true;
	}

	bool _PCrecv::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCrecv::check(void)
	{
		return m_pBpStreamIn && m_pPCLout && _ReferenceFrame::check();
	}

	void _PCrecv::clear(void)
	{
		if (m_pPCLout)
		{
			m_pPCLout->set({});
		}
	}

	void _PCrecv::update(void)
	{
		vector<BYTE_PACKET> vPackets;
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!check())
			{
				m_vPacket.clear();
				m_vPendingPoints.clear();
				m_tPendingStamp = 0;
				continue;
			}

			m_pBpStreamIn->get(vPackets, m_tLastBpStreamIn);
			for (const BYTE_PACKET &packet : vPackets)
			{
				for (uint8_t byte : packet.m_vB)
				{
					inputByte(byte);
				}
				m_tLastBpStreamIn = packet.m_tStamp;
			}
		}
	}

	void _PCrecv::inputByte(uint8_t byte)
	{
		if (m_vPacket.size() < sizeof(pcstream::magic))
		{
			if (byte != pcstream::magic[m_vPacket.size()])
			{
				m_vPacket.clear();
				if (byte == pcstream::magic[0])
				{
					m_vPacket.push_back(byte);
				}
				return;
			}
		}
		m_vPacket.push_back(byte);
		if (m_vPacket.size() == 8)
		{
			m_nPacketBytes = pcstream::unpackUint(m_vPacket.data() + 4, 4);
			if (m_nPacketBytes < pcstream::headerBytes || m_nPacketBytes > pcstream::maxPacketBytes ||
				(m_nPacketBytes - pcstream::headerBytes) % pcstream::pointBytes != 0)
			{
				m_vPacket.clear();
				m_vPendingPoints.clear();
				m_tPendingStamp = 0;
				return;
			}
		}
		if (m_vPacket.size() >= pcstream::headerBytes && m_vPacket.size() == m_nPacketBytes)
		{
			decodePacket();
			m_vPacket.clear();
		}
	}

	void _PCrecv::decodePacket(void)
	{
		const uint8_t *pBytes = m_vPacket.data();
		const uint64_t stamp = pcstream::unpackUint(pBytes + 8, 8);
		const uint32_t total = pcstream::unpackUint(pBytes + 16, 4);
		const uint32_t first = pcstream::unpackUint(pBytes + 20, 4);
		const size_t count = (m_vPacket.size() - pcstream::headerBytes) / pcstream::pointBytes;
		if (stamp == 0 || total > m_nPmax || first > total || count > total - first ||
			(count == 0 && total != 0))
		{
			m_vPendingPoints.clear();
			m_tPendingStamp = 0;
			return;
		}

		if (first == 0)
		{
			m_vPendingPoints.clear();
			m_vPendingPoints.reserve(total);
			m_tPendingStamp = stamp;
			m_nPendingPoints = total;
		}
		if (stamp != m_tPendingStamp ||
			total != m_nPendingPoints || first != m_vPendingPoints.size())
		{
			m_vPendingPoints.clear();
			m_tPendingStamp = 0;
			return;
		}

		for (size_t i = 0; i < count; ++i)
		{
			const uint8_t *pPoint = pBytes + pcstream::headerBytes + i * pcstream::pointBytes;
			GEOMETRY_POINT point;
			for (int axis = 0; axis < 3; ++axis)
			{
				point.m_vP[axis] = pcstream::unpackFloat(pPoint + axis * 4);
				point.m_vC[axis] = pcstream::unpackFloat(pPoint + 12 + axis * 4);
			}
			point.m_vP = m_mPosef * point.m_vP;
			point.m_tStamp = pcstream::unpackUint(pPoint + 24, 8);
			m_vPendingPoints.push_back(point);
		}
		if (m_vPendingPoints.size() == total)
		{
			m_pPCLout->set(m_vPendingPoints, stamp);
			m_vPendingPoints.clear();
			m_tPendingStamp = 0;
		}
	}
}
