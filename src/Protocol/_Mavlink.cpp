#include "_Mavlink.h"
#include <unordered_set>

namespace kai
{
	std::mutex _Mavlink::m_mavlinkMutex;

	_Mavlink::_Mavlink()
	{
	}

	_Mavlink::~_Mavlink()
	{
		stop();
		DEL(m_pTr);
	}

	bool _Mavlink::loadConfig(void)
	{
		stop();
		IF_F(!this->_ModuleBase::loadConfig());
		json &j = *m_pJ;
		if (!j.contains("threadR"))
		{
			j["threadR"] = {{"FPS", m_pT->getTargetFPS()}};
		}
		DEL(m_pTr);
		m_pTr = createThread(jK(j, "threadR"), "threadR");
		NULL_F(m_pTr);

		jKv(j, "mySystemID", m_mySystemID);
		jKv(j, "myComponentID", m_myComponentID);
		jKv(j, "myType", m_myType);

		int devSystemID = m_devSystemID.load();
		int devComponentID = m_devComponentID.load();
		jKv(j, "devSystemID", devSystemID);
		jKv(j, "devComponentID", devComponentID);
		m_devSystemID = devSystemID;
		m_devComponentID = devComponentID;
		jKv(j, "devType", m_devType);

		jKv(j, "iMavComm", m_iMavComm);

		m_status = {};
		m_nDroppedPackets = 0;

		return true;
	}

	bool _Mavlink::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		IF_F(m_pTr && !m_pTr->saveConfig(false));

		json &j = *m_pJ;
		j["mySystemID"] = m_mySystemID;
		j["myComponentID"] = m_myComponentID;
		j["myType"] = m_myType;
		j["devSystemID"] = m_devSystemID.load();
		j["devComponentID"] = m_devComponentID.load();
		j["devType"] = m_devType;
		j["iMavComm"] = m_iMavComm;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Mavlink::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n;

		jKv(j, "BytePacketStreamIn", n);
		m_pBpStreamIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pBpStreamIn, "BytePacketStreamIn not found: " + n);

		n.clear();
		jKv(j, "BytePacketStreamOut", n);
		m_pBpStreamOut = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pBpStreamOut, "BytePacketStreamOut not found: " + n);

		n.clear();
		jKv(j, "MavlinkStreamIn", n);
		m_pMavStreamIn = dynamic_cast<MavlinkStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pMavStreamIn, "MavlinkStreamIn not found: " + n);

		n.clear();
		jKv(j, "MavlinkStreamOut", n);
		m_pMavStreamOut = dynamic_cast<MavlinkStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pMavStreamOut, "MavlinkStreamOut not found: " + n);

		return true;
	}

	bool _Mavlink::start(void)
	{
		IF_F(!m_pT || !m_pTr || !bStopped());
		if (!m_pT->startThread(getUpdateW, this) ||
			!m_pTr->startThread(getUpdateR, this))
		{
			stop();
			return false;
		}
		return true;
	}

	void _Mavlink::pause(void)
	{
		if (m_pT) m_pT->pause();
		if (m_pTr) m_pTr->pause();
	}

	void _Mavlink::resume(void)
	{
		if (m_pT) m_pT->run();
		if (m_pTr) m_pTr->run();
	}

	void _Mavlink::stop(void)
	{
		// Wake both workers before waiting for either to finish.
		if (m_pT) m_pT->stop();
		if (m_pTr) m_pTr->stop();
		if (m_pT) m_pT->join();
		if (m_pTr) m_pTr->join();
	}

	bool _Mavlink::bRun(void)
	{
		return (m_pT && m_pT->bRun()) || (m_pTr && m_pTr->bRun());
	}

	bool _Mavlink::bRunning(void)
	{
		return (m_pT && m_pT->bRunning()) || (m_pTr && m_pTr->bRunning());
	}

	bool _Mavlink::bStopped(void)
	{
		return (!m_pT || m_pT->bStopped()) && (!m_pTr || m_pTr->bStopped());
	}

	_Thread *_Mavlink::getThread(const string &name)
	{
		if (m_pTr && m_pTr->getName() == name)
			return m_pTr;

		return _ModuleBase::getThread(name);
	}

	bool _Mavlink::check(void)
	{
		NULL_F(m_pTr);
		IF_F(!m_pMavStreamIn);
		IF_F(!m_pMavStreamOut);
		IF_F(!m_pBpStreamIn);
		IF_F(!m_pBpStreamOut);

		return this->_ModuleBase::check();
	}

	void _Mavlink::updateW(void)
	{
		vector<MavMsgBase *> vMsg;
		std::unordered_set<MavMsgBase *> sent;

		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun()) break;
			IF_CONT(!m_pMavStreamIn || !m_pBpStreamOut);

			m_pMavStreamIn->getMsgQueue(vMsg, m_tLastMavStreamIn);
			sent.clear();
			uint64_t tLast = m_tLastMavStreamIn;
			bool bComplete = true;
			for (MavMsgBase *pM : vMsg)
			{
				if (m_pT->bOnPause()) m_pT->autoFPS();
				if (!m_pT->bRun())
				{
					bComplete = false;
					break;
				}
				// The queue can retain multiple entries pointing to one updated message.
				IF_CONT(!pM || !sent.insert(pM).second);

				const uint64_t tStamp = pM->getTstamp();
				mavlink_message_t msg;
				{
					std::lock_guard<std::mutex> lock(m_mavlinkMutex);
					msg = pM->encode(m_mySystemID, m_myComponentID,
										m_devSystemID.load(), m_devComponentID.load());
				}
				if (!writeMessage(msg))
				{
					bComplete = false;
					break;
				}
				tLast = std::max(tLast, tStamp);
			}

			// Commit only complete batches: timestamps need not follow queue order.
			// Clearing the shared queue could erase concurrent arrivals.
			if (bComplete) m_tLastMavStreamIn = tLast;
		}
	}

	void _Mavlink::updateR(void)
	{
		mavlink_message_t msg;

		while (m_pTr->bRun())
		{
			m_pTr->autoFPS();
			if (!m_pTr->bRun()) break;
			IF_CONT(!m_pMavStreamOut || !readMessage(&msg));
			m_pTr->skipSleep();

			{
				// Publish both learned IDs together before the sender encodes a message.
				std::lock_guard<std::mutex> lock(m_mavlinkMutex);
				if (m_devSystemID < 0)
					m_devSystemID = msg.sysid;

				if (m_devComponentID < 0)
					m_devComponentID = msg.compid;
			}

			// Ignore messages from other senders.
			IF_CONT(msg.sysid != m_devSystemID);
			IF_CONT(msg.compid != m_devComponentID);

			m_pMavStreamOut->decode(msg);
		}
	}

	bool _Mavlink::readMessage(mavlink_message_t *pMsg)
	{
		NULL_F(m_pBpStreamIn);
		NULL_F(pMsg);

		if (m_iPacketIn == m_vPacketIn.size())
		{
			m_pBpStreamIn->getPackets(m_vPacketIn, m_tLastBpStreamIn);
			m_iPacketIn = 0;
			m_iByteIn = 0;
		}

		std::lock_guard<std::mutex> lock(m_mavlinkMutex);
		while (m_iPacketIn < m_vPacketIn.size())
		{
			const BYTE_PACKET &packet = m_vPacketIn[m_iPacketIn];
			m_tLastBpStreamIn = packet.m_tStamp;

			while (m_iByteIn < packet.m_vB.size())
			{
				uint8_t result = mavlink_frame_char(m_iMavComm,
													packet.m_vB[m_iByteIn++],
													pMsg,
													&m_status);
				m_nDroppedPackets = m_status.packet_rx_drop_count;
				IF__(result == 1, true);

				if (result == 2)
				{
					LOG_I(" -> DROPPED PACKETS:" + i2str(m_status.packet_rx_drop_count));
				}
			}

			m_iPacketIn++;
			m_iByteIn = 0;
		}

		return false;
	}

	bool _Mavlink::writeMessage(const mavlink_message_t &msg)
	{
		NULL_F(m_pBpStreamOut);

		uint8_t pB[MAVLINK_MAX_PACKET_LEN];
		int nB = mavlink_msg_to_send_buffer(pB, &msg);
		m_pBpStreamOut->addPacket(vector<uint8_t>(pB, pB + nB));

		LOG_I("<- Wrote MSG_ID = " + i2str((int)msg.msgid) + ", seq = " + i2str((int)msg.seq));
		return true;
	}

	void _Mavlink::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		if (m_pTr) m_pTr->console(pConsole);
		IF_(!check());

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("BytePacketStream linked", 0);
		pC->addMsg("mySysID = " + i2str(m_mySystemID) + " myComID = " + i2str(m_myComponentID) + " myType = " + i2str(m_myType));
		pC->addMsg("devSysID = " + i2str(m_devSystemID.load()) + " devComID = " + i2str(m_devComponentID.load()) + " devType = " + i2str(m_devType));
		pC->addMsg("Dropped packets = " + i2str(m_nDroppedPackets.load()));
	}

}
