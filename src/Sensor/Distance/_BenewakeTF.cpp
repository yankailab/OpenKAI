/*
 *  Created on: July 24, 2021
 *      Author: yankai
 */
#include "_BenewakeTF.h"
#include "DistancePoint.h"
#include "../../UI/_Console.h"

namespace kai
{

	_BenewakeTF::_BenewakeTF()
	{
	}

	_BenewakeTF::~_BenewakeTF()
	{
		if (m_pT)
		{
			m_pT->join();
		}
		DEL_ARRAY(m_frame.m_pB);
	}

	bool _BenewakeTF::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv<float>(j, "vRange", m_vRange);
		IF_F(!m_vRange.allFinite() || m_vRange.x() < 0 || m_vRange.x() >= m_vRange.y());

		m_frame.init(9);

		return true;
	}

	bool _BenewakeTF::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		json &j = *m_pJ;
		j["vRange"] = {m_vRange.x(), m_vRange.y()};

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _BenewakeTF::link(InstanceMgr *pM)
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

	bool _BenewakeTF::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _BenewakeTF::update(void)
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

	bool _BenewakeTF::readByte(uint8_t *pB)
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

	bool _BenewakeTF::readCMD(void)
	{
		uint8_t b;

		while (readByte(&b))
		{
			if (m_frame.m_header != 0)
			{
				m_frame.m_pB[m_frame.m_iB] = b;
				m_frame.m_iB++;

				IF__(m_frame.m_iB >= 9, true);
			}
			else if (b == BENEWAKE_BEGIN)
			{
				m_frame.m_header = b;
				m_frame.m_pB[0] = b;
				m_frame.m_iB = 1;
			}
		}

		return false;
	}

	void _BenewakeTF::handleCMD(void)
	{
		bool bValid = m_frame.m_pB[1] == BENEWAKE_BEGIN && verifyCheckSum(m_frame.m_pB, 9);
		uint16_t d = m_frame.m_pB[2] | (m_frame.m_pB[3] << 8);
		uint16_t a = m_frame.m_pB[4] | (m_frame.m_pB[5] << 8);
		m_frame.reset();
		IF_(!bValid || !m_pPCLout);

		uint64_t tStamp = getTns();
		GEOMETRY_POINT point = distancePoint(d * 0.01f, a, 0, 1, 0, m_vRange, tStamp);
		m_pPCLout->set({point}, tStamp);
	}

	uint8_t _BenewakeTF::verifyCheckSum(const uint8_t *data, size_t dataLength)
	{
		uint8_t sum = 0;
		for (size_t i = 0; i < dataLength - 1; i++)
		{
			sum += data[i];
		}
		return sum == data[dataLength - 1];
	}

	void _BenewakeTF::console(void *pConsole)
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
