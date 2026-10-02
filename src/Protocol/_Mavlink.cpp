#include "_Mavlink.h"

namespace kai
{
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

		jKv(j, "devSystemID", m_devSystemID);
		jKv(j, "devComponentID", m_devComponentID);
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
		j["devSystemID"] = m_devSystemID;
		j["devComponentID"] = m_devComponentID;
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
		vector<std::shared_ptr<MavMsgBase>> vMsg;

		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			IF_CONT(!m_pMavStreamIn || !m_pBpStreamOut);

			m_pMavStreamIn->getMsgQueue(vMsg, m_tLastMavStreamIn);
			uint64_t tLast = m_tLastMavStreamIn;

			bool bComplete = true;
			for (const auto &pM : vMsg)
			{
				const uint64_t tStamp = pM->getTstamp();
				mavlink_message_t msg;
				msg = pM->encode(m_mySystemID, m_myComponentID,
								 m_devSystemID, m_devComponentID);

				if (!writeMessage(msg))
				{
					bComplete = false;
					break;
				}

				tLast = std::max(tLast, tStamp);
			}

			// Commit only complete batches; readers retain independent cursors.
			// Clearing the shared queue could erase concurrent arrivals.
			if (bComplete)
			{
				m_tLastMavStreamIn = tLast;
			}
		}
	}

	bool _Mavlink::writeMessage(const mavlink_message_t &msg)
	{
		NULL_F(m_pBpStreamOut);

		uint8_t pB[MAVLINK_MAX_PACKET_LEN];
		int nB = mavlink_msg_to_send_buffer(pB, &msg);
		m_pBpStreamOut->addPacket(vector<uint8_t>(pB, pB + nB));

		LOG_I("Packet added, MSG_ID = " + i2str((int)msg.msgid) + ", seq = " + i2str((int)msg.seq));
		return true;
	}

	void _Mavlink::updateR(void)
	{
		mavlink_message_t msg;

		while (m_pTr->bRun())
		{
			m_pTr->autoFPS();

			IF_CONT(!m_pMavStreamOut);

			IF_CONT(!readMessage(&msg));
			m_pTr->skipSleep();

			if (m_devSystemID < 0)
				m_devSystemID = msg.sysid;

			if (m_devComponentID < 0)
				m_devComponentID = msg.compid;

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

	void _Mavlink::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		if (m_pTr)
			m_pTr->console(pConsole);
		IF_(!check());

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("BytePacketStream linked", 0);
		pC->addMsg("mySysID = " + i2str(m_mySystemID) + " myComID = " + i2str(m_myComponentID) + " myType = " + i2str(m_myType));
		pC->addMsg("devSysID = " + i2str(m_devSystemID) + " devComID = " + i2str(m_devComponentID) + " devType = " + i2str(m_devType));
		pC->addMsg("Dropped packets = " + i2str(m_nDroppedPackets.load()));
	}

}
