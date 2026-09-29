/*
 * _PCio.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#include "_PCsend.h"

namespace kai
{
	_PCsend::_PCsend()
	{
	}

	_PCsend::~_PCsend()
	{
	}

	bool _PCsend::loadConfig(void)
	{
		IF_F(!_ReferenceFrame::loadConfig());
		jKv(*m_pJ, "tInt", m_tInt);
		jKv(*m_pJ, "nB", m_nB);
		IF_Le_F(m_nB < int(pcstream::headerBytes + pcstream::pointBytes) ||
			m_nB > int(pcstream::maxPacketBytes), "Invalid PCL2 packet size nB");
		m_vPacket.resize(m_nB);
		return true;
	}

	bool _PCsend::saveConfig(bool bExport)
	{
		IF_F(!_ReferenceFrame::saveConfig(false));
		(*m_pJ)["tInt"] = m_tInt;
		(*m_pJ)["nB"] = m_nB;
		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _PCsend::link(InstanceMgr *pM)
	{
		IF_F(!_ReferenceFrame::link(pM));
		string name;
		jKv(*m_pJ, "_IObase", name);
		m_pIO = static_cast<_IObase *>(pM->findModule(name));
		IF_Le_F(!m_pIO, "_IObase not found: " + name);

		name.clear();
		jKv(*m_pJ, "PCLframeIn", name);
		m_pPCLin = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(name)));
		IF_Le_F(!m_pPCLin, "PCLframeIn not found: " + name);
		m_tInput = 0;
		return true;
	}

	bool _PCsend::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCsend::check(void)
	{
		return m_pIO && m_pIO->bOpen() && m_pPCLin && !m_vPacket.empty() && _ReferenceFrame::check();
	}

	void _PCsend::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			sendPC();
		}
	}

	void _PCsend::sendPC(void)
	{
		if (!check())
		{
			m_tInput = 0;
			return;
		}

		if (m_pPCLin->getTstamp() == m_tInput)
		{
			return;
		}
		const uint64_t stamp = m_pPCLin->get(m_vPoints);
		if (stamp == m_tInput || m_vPoints.size() > UINT32_MAX)
		{
			return;
		}

		const size_t pointsPerPacket = (m_vPacket.size() - pcstream::headerBytes) / pcstream::pointBytes;
		size_t first = 0;
		do
		{
			const size_t count = std::min(pointsPerPacket, m_vPoints.size() - first);
			const size_t bytes = pcstream::headerBytes + count * pcstream::pointBytes;
			uint8_t *pBytes = m_vPacket.data();
			std::memcpy(pBytes, pcstream::magic, sizeof(pcstream::magic));
			pcstream::packUint(pBytes + 4, bytes, 4);
			pcstream::packUint(pBytes + 8, stamp, 8);
			pcstream::packUint(pBytes + 16, m_vPoints.size(), 4);
			pcstream::packUint(pBytes + 20, first, 4);
			for (size_t i = 0; i < count; ++i)
			{
				const GEOMETRY_POINT &point = m_vPoints[first + i];
				uint8_t *pPoint = pBytes + pcstream::headerBytes + i * pcstream::pointBytes;
				for (int axis = 0; axis < 3; ++axis)
				{
					pcstream::packFloat(pPoint + axis * 4, point.m_vP[axis]);
					pcstream::packFloat(pPoint + 12 + axis * 4, point.m_vC[axis]);
				}
				pcstream::packUint(pPoint + 24, point.m_tStamp, 8);
			}

			// On failure retry the entire frame next iteration. Its first
			// packet resets any incomplete frame at the receiver.
			if (!m_pIO->write(pBytes, static_cast<int>(bytes)))
			{
				return;
			}
			first += count;
		}
		while (first < m_vPoints.size() && m_pT->bRun());

		if (first == m_vPoints.size())
		{
			m_tInput = stamp;
		}
		if (m_tInt > 0)
		{
			m_pT->sleepT(m_tInt);
		}
	}
}
