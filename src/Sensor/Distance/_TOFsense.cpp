/*
 *  Created on: July 15, 2020
 *      Author: yankai
 */
#include "_TOFsense.h"
#include "DistancePoint.h"
#include "../../UI/_Console.h"

namespace kai
{

	_TOFsense::_TOFsense()
	{
	}

	_TOFsense::~_TOFsense()
	{
		if (m_pT)
		{
			m_pT->join();
		}
		DEL_ARRAY(m_frame.m_pB);
	}

	bool _TOFsense::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv<float>(j, "vRange", m_vRange);
		IF_F(!m_vRange.allFinite() || m_vRange.x() < 0 || m_vRange.x() >= m_vRange.y());

		jKv(j, "nDiv", m_nDiv);
		IF_F(m_nDiv <= 0 || m_nDiv > 256);
		jKv(j, "fovH", m_fovH);
		IF_F(!std::isfinite(m_fovH) || m_fovH <= 0 || m_fovH > 360);
		m_vPoints.assign(m_nDiv, GEOMETRY_POINT());

		m_frame.init(16);

		return true;
	}

	bool _TOFsense::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		json &j = *m_pJ;
		j["vRange"] = {m_vRange.x(), m_vRange.y()};
		j["nDiv"] = m_nDiv;
		j["fovH"] = m_fovH;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _TOFsense::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "BytePacketStreamIn", n);
		m_pBpStreamIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pBpStreamIn, "BytePacketStreamIn not found: " + n);

		n.clear();
		jKv(j, "PCLframeOut", n);
		m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pPCLout, "PCLframeOut not found: " + n);

		return true;
	}

	bool _TOFsense::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _TOFsense::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_pBpStreamIn || !m_pPCLout)
			{
				m_pT->sleepT(NSEC_SEC);
				continue;
			}

			m_pT->autoFPS();

			while (readCMD())
			{
				handleCMD();
			}
		}
	}

	bool _TOFsense::readByte(uint8_t *pB)
	{
		NULL_F(m_pBpStreamIn);
		if (m_iByteIn == m_vBytesIn.size())
		{
			vector<BYTE_PACKET> vPacket;
			m_pBpStreamIn->get(vPacket, m_tLastBpStreamIn);
			m_vBytesIn.clear();
			m_iByteIn = 0;
			for (const BYTE_PACKET &packet : vPacket)
			{
				m_vBytesIn.insert(m_vBytesIn.end(), packet.m_vB.begin(), packet.m_vB.end());
				m_tLastBpStreamIn = packet.m_tStamp;
			}
		}

		IF_F(m_iByteIn == m_vBytesIn.size());
		*pB = m_vBytesIn[m_iByteIn++];
		return true;
	}

	bool _TOFsense::readCMD(void)
	{
		uint8_t b;

		while (readByte(&b))
		{
			if (m_frame.m_header != 0)
			{
				m_frame.m_pB[m_frame.m_iB] = b;
				m_frame.m_iB++;

				if (m_frame.m_iB == 2)
				{
					// m_frame.m_nPayload = ;
				}
				else
				{
					IF__(m_frame.m_iB >= 16, true);
				}
			}
			else if (b == NLINK_BEGIN)
			{
				m_frame.m_header = b;
				m_frame.m_pB[0] = b;
				m_frame.m_iB = 1;
			}
		}

		return false;
	}

	void _TOFsense::handleCMD(void)
	{
		uint8_t cs = verifyCheckSum(m_frame.m_pB, 16);
		TOFSENSE_SENSOR rf;
		memcpy(&rf, m_frame.m_pB, 16);
		m_frame.reset();
		IF_(!cs);

		IF_(!m_pPCLout || rf.id >= m_vPoints.size());

		uint64_t tStamp = getTns();
		float d = int24Value((uint8_t *)&rf.distance) * 0.001f;
		if (rf.distanceStatus != 0)
		{
			d = -1;
		}
		m_vPoints[rf.id] = distancePoint(d, rf.signalStrength, rf.id, m_nDiv,
			m_fovH, m_vRange, tStamp);
		m_pPCLout->set(m_vPoints, tStamp);
	}

	int32_t _TOFsense::int24Value(uint8_t *pD)
	{
		NULL__(pD, -1);

		int32_t value = pD[0] | (pD[1] << 8) | (pD[2] << 16);
		if (value & 0x800000)
		{
			value -= 0x1000000;
		}
		return value;
	}

	uint8_t _TOFsense::verifyCheckSum(const uint8_t *data, size_t dataLength)
	{
		uint8_t sum = 0;
		for (size_t i = 0; i < dataLength - 1; i++)
		{
			sum += data[i];
		}
		return sum == data[dataLength - 1];
	}

	void _TOFsense::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		NULL_(m_pPCLout);
		vector<GEOMETRY_POINT> points;
		m_pPCLout->get(points);
		string msg = "nDiv=" + i2str(points.size());
		for (const GEOMETRY_POINT &point : points)
		{
			float d = point.m_tStamp ? point.m_vP.norm() : -1;
			msg += " | " + f2str(d) + " (" + f2str(point.m_vC.x()) + ")";
		}
		((_Console *)pConsole)->addMsg(msg + " |");
	}

}
