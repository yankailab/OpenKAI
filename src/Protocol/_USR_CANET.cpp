#include "_USR_CANET.h"
#include "../UI/_Console.h"

namespace kai
{

	_USR_CANET::_USR_CANET()
	{
	}

	_USR_CANET::~_USR_CANET()
	{
		stop();
		DEL(m_pTr);
	}

	bool _USR_CANET::loadConfig(void)
	{
		stop();
		IF_F(!_ModuleBase::loadConfig());
		json &j = *m_pJ;

		if (!j.contains("threadR"))
		{
			j["threadR"] = {{"FPS", m_pT->getTargetFPS()}};
		}

		DEL(m_pTr);
		m_pTr = createThread(jK(j, "threadR"), "threadR");
		return m_pTr != nullptr;
	}

	bool _USR_CANET::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		IF_F(m_pTr && !m_pTr->saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _USR_CANET::link(InstanceMgr *pM)
	{
		IF_F(!_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n;
		jKv(j, "BytePacketStreamIn", n);
		m_pBpStreamIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pBpStreamIn, "BytePacketStreamIn not found: " + n);

		n.clear();
		jKv(j, "BytePacketStreamOut", n);
		m_pBpStreamOut = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pBpStreamOut, "BytePacketStreamOut not found: " + n);

		n.clear();
		jKv(j, "CANframeStreamIn", n);
		m_pCANframeIn = dynamic_cast<CANframeStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pCANframeIn, "CANframeStreamIn not found: " + n);

		n.clear();
		jKv(j, "CANframeStreamOut", n);
		m_pCANframeOut = dynamic_cast<CANframeStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pCANframeOut, "CANframeStreamOut not found: " + n);
		IF_Le_F((m_pCANframeIn != nullptr) != (m_pBpStreamOut != nullptr),
			"CANframeStreamIn and BytePacketStreamOut must be configured together");
		IF_Le_F((m_pBpStreamIn != nullptr) != (m_pCANframeOut != nullptr),
			"BytePacketStreamIn and CANframeStreamOut must be configured together");
		IF_Le_F(!bOpen(), "No CAN frame streams configured");

		m_tLastCANframeIn = 0;
		m_tLastBpStreamIn = 0;
		m_vFrameBytes.clear();
		return true;
	}

	bool _USR_CANET::open(void)
	{
		return bOpen();
	}

	bool _USR_CANET::bOpen(void)
	{
		return ((m_pCANframeIn != nullptr) == (m_pBpStreamOut != nullptr)) &&
			((m_pBpStreamIn != nullptr) == (m_pCANframeOut != nullptr)) &&
			(m_pCANframeIn || m_pCANframeOut);
	}

	void _USR_CANET::close(void)
	{
		// The linked byte streams own the transport connection.
	}

	bool _USR_CANET::start(void)
	{
		IF_F(!check() || !bStopped());
		if (!m_pT->startThread(getUpdateW, this) ||
			!m_pTr->startThread(getUpdateR, this))
		{
			stop();
			return false;
		}
		return true;
	}

	bool _USR_CANET::check(void)
	{
		IF_F(!m_pTr || !bOpen());
		return _ModuleBase::check();
	}

	bool _USR_CANET::bRun(void)
	{
		return (m_pT && m_pT->bRun()) || (m_pTr && m_pTr->bRun());
	}

	bool _USR_CANET::bRunning(void)
	{
		return (m_pT && m_pT->bRunning()) || (m_pTr && m_pTr->bRunning());
	}

	bool _USR_CANET::bStopped(void)
	{
		return (!m_pT || m_pT->bStopped()) && (!m_pTr || m_pTr->bStopped());
	}

	void _USR_CANET::pause(void)
	{
		if (m_pT)
		{
			m_pT->pause();
		}
		if (m_pTr)
		{
			m_pTr->pause();
		}
	}

	void _USR_CANET::resume(void)
	{
		if (m_pT)
		{
			m_pT->run();
		}
		if (m_pTr)
		{
			m_pTr->run();
		}
	}

	void _USR_CANET::stop(void)
	{
		// Wake both workers before waiting for either to finish.
		if (m_pT)
		{
			m_pT->stop();
		}
		if (m_pTr)
		{
			m_pTr->stop();
		}
		if (m_pT)
		{
			m_pT->join();
		}
		if (m_pTr)
		{
			m_pTr->join();
		}
	}

	_Thread *_USR_CANET::getThread(const string &name)
	{
		if (m_pTr && m_pTr->getName() == name)
		{
			return m_pTr;
		}
		return _ModuleBase::getThread(name);
	}

	void _USR_CANET::updateW(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun())
			{
				break;
			}
			sendFrame();
		}
	}

	void _USR_CANET::updateR(void)
	{
		while (m_pTr->bRun())
		{
			m_pTr->autoFPS();
			if (!m_pTr->bRun())
			{
				break;
			}
			readFrame();
		}
	}

	bool _USR_CANET::sendFrame(void)
	{
		IF_F(!check() || !m_pCANframeIn || !m_pBpStreamOut);

		vector<CAN_FRAME> vFrames;
		m_pCANframeIn->get(vFrames, m_tLastCANframeIn);
		bool bSent = false;
		for (const CAN_FRAME &f : vFrames)
		{
			// The stream timestamp can be newer than its individual frames.
			m_tLastCANframeIn = std::max(m_tLastCANframeIn, f.m_tStamp);
			IF_CONT(f.m_nData > 8 || f.m_ID > (f.m_bExtended ? 0x1FFFFFFFU : 0x7FFU));

			vector<uint8_t> vBytes(CANET_BUF_N, 0);
			vBytes[0] = f.m_nData;
			if (f.m_bExtended)
			{
				vBytes[0] |= (1 << 7);
			}
			if (f.m_bRTR)
			{
				vBytes[0] |= (1 << 6);
			}

			// USR CANET identifiers use network (big-endian) byte order.
			vBytes[1] = (f.m_ID >> 24) & 0xFF;
			vBytes[2] = (f.m_ID >> 16) & 0xFF;
			vBytes[3] = (f.m_ID >> 8) & 0xFF;
			vBytes[4] = f.m_ID & 0xFF;
			memcpy(&vBytes[5], f.m_pData, f.m_nData);
			m_pBpStreamOut->add({{vBytes, getTns()}});
			bSent = true;

			LOG_I("Sent: id=" + i2str(f.m_ID) + ", len=" + i2str(f.m_nData));
		}
		return bSent;
	}

	bool _USR_CANET::readFrame(void)
	{
		IF_F(!check() || !m_pBpStreamIn || !m_pCANframeOut);

		vector<BYTE_PACKET> vPackets;
		m_pBpStreamIn->get(vPackets, m_tLastBpStreamIn);
		for (const BYTE_PACKET &packet : vPackets)
		{
			m_vFrameBytes.insert(m_vFrameBytes.end(), packet.m_vB.begin(), packet.m_vB.end());
			m_tLastBpStreamIn = std::max(m_tLastBpStreamIn, packet.m_tStamp);
		}

		vector<CAN_FRAME> vFrames;
		size_t nConsumed = 0;
		while (m_vFrameBytes.size() - nConsumed >= CANET_BUF_N)
		{
			const uint8_t *pB = m_vFrameBytes.data() + nConsumed;
			nConsumed += CANET_BUF_N;

			CAN_FRAME f;
			f.m_bExtended = (pB[0] & (1 << 7)) != 0;
			f.m_bRTR = (pB[0] & (1 << 6)) != 0;
			f.m_nData = pB[0] & 0x0F;
			f.m_ID = (uint32_t(pB[1]) << 24) | (uint32_t(pB[2]) << 16) |
				(uint32_t(pB[3]) << 8) | uint32_t(pB[4]);
			// Every record occupies 13 bytes, even if its control byte is invalid.
			IF_CONT((pB[0] & 0x30) || f.m_nData > 8 ||
				f.m_ID > (f.m_bExtended ? 0x1FFFFFFFU : 0x7FFU));

			memcpy(f.m_pData, &pB[5], f.m_nData);
			m_tLastCANframeOut = std::max(getTns(), m_tLastCANframeOut + 1);
			f.m_tStamp = m_tLastCANframeOut;
			vFrames.push_back(f);

			LOG_I("Recv: id=" + i2str(f.m_ID) + ", len=" + i2str(f.m_nData));
		}

		if (nConsumed > 0)
		{
			m_vFrameBytes.erase(m_vFrameBytes.begin(), m_vFrameBytes.begin() + nConsumed);
		}
		IF_F(vFrames.empty());
		m_pCANframeOut->add(vFrames, m_tLastCANframeOut);
		m_nFrameRecv += vFrames.size();
		return true;
	}

	void _USR_CANET::console(void *pConsole)
	{
		NULL_(pConsole);
		_ModuleBase::console(pConsole);
		if (m_pTr)
		{
			m_pTr->console(pConsole);
		}

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("bOpen = " + i2str(bOpen()), 1);
		pC->addMsg("nFrameRecv = " + i2str(m_nFrameRecv.load()), 1);
	}

}
