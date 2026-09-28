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
		IF_F(!_GeometryBase::loadConfig());
		jKv(*m_pJ, "tInt", m_tInt);
		jKv(*m_pJ, "nB", m_nB);
		IF_Le_F(m_nB < int(pcstream::headerBytes + pcstream::pointBytes) ||
			m_nB > int(pcstream::maxPacketBytes), "Invalid PCL1 packet size nB");
		m_vPacket.resize(m_nB);
		return true;
	}

	bool _PCsend::saveConfig(bool bExport)
	{
		IF_F(!_GeometryBase::saveConfig(false));
		(*m_pJ)["tInt"] = m_tInt;
		(*m_pJ)["nB"] = m_nB;
		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _PCsend::link(InstanceMgr *pM)
	{
		IF_F(!_GeometryBase::link(pM));
		string name;
		jKv(*m_pJ, "_IObase", name);
		m_pIO = static_cast<_IObase *>(pM->findModule(name));
		IF_Le_F(!m_pIO, "_IObase not found: " + name);

		name.clear();
		jKv(*m_pJ, "PCLframeIn", name);
		m_pPCLin = dynamic_cast<PCLframe *>(static_cast<DataStreamBase *>(pM->findDataStream(name)));
		IF_Le_F(!m_pPCLin, "PCLframeIn not found: " + name);
		m_inputRevision = 0;
		return true;
	}

	bool _PCsend::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCsend::check(void)
	{
		return m_pIO && m_pIO->bOpen() && m_pPCLin && !m_vPacket.empty() && _GeometryBase::check();
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
			m_inputRevision = 0;
			return;
		}

		const PCLframe::SnapshotPtr frame = m_pPCLin->get();
		if (frame->m_revision == m_inputRevision || frame->m_vPoints.size() > UINT32_MAX)
		{
			return;
		}

		const size_t pointsPerPacket = (m_vPacket.size() - pcstream::headerBytes) / pcstream::pointBytes;
		size_t first = 0;
		do
		{
			const size_t count = std::min(pointsPerPacket, frame->m_vPoints.size() - first);
			const size_t bytes = pcstream::headerBytes + count * pcstream::pointBytes;
			uint8_t *pBytes = m_vPacket.data();
			std::memcpy(pBytes, pcstream::magic, sizeof(pcstream::magic));
			pcstream::packUint(pBytes + 4, bytes, 4);
			pcstream::packUint(pBytes + 8, frame->m_revision, 8);
			pcstream::packUint(pBytes + 16, frame->m_tStamp, 8);
			pcstream::packUint(pBytes + 24, frame->m_vPoints.size(), 4);
			pcstream::packUint(pBytes + 28, first, 4);
			for (size_t i = 0; i < count; ++i)
			{
				const GEOMETRY_POINT &point = frame->m_vPoints[first + i];
				uint8_t *pPoint = pBytes + pcstream::headerBytes + i * pcstream::pointBytes;
				for (int axis = 0; axis < 3; ++axis)
				{
					pcstream::packFloat(pPoint + axis * 4, point.m_vP[axis]);
					pcstream::packFloat(pPoint + 12 + axis * 4, point.m_vC[axis]);
				}
				pcstream::packUint(pPoint + 24, point.m_tStamp, 8);
			}

			// On failure retry the entire snapshot next iteration. Its first
			// packet resets any incomplete frame at the receiver.
			if (!m_pIO->write(pBytes, static_cast<int>(bytes)))
			{
				return;
			}
			first += count;
		}
		while (first < frame->m_vPoints.size() && m_pT->bRun());

		if (first == frame->m_vPoints.size())
		{
			m_inputRevision = frame->m_revision;
		}
		if (m_tInt > 0)
		{
			m_pT->sleepT(m_tInt);
		}
	}
}
