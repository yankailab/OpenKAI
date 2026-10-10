#include "_APmav_RTCM.h"

namespace kai
{

	_APmav_RTCM::_APmav_RTCM()
	{
	}

	_APmav_RTCM::~_APmav_RTCM()
	{
	}

	bool _APmav_RTCM::loadConfig(void)
	{
		IF_F(!this->_RTCMcast::loadConfig());

		return true;
	}

	bool _APmav_RTCM::saveConfig(bool bExport)
	{
		IF_F(!_RTCMcast::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _APmav_RTCM::link(InstanceMgr *pM)
	{
		IF_F(!this->_ProtocolBase::link(pM));	// RTCM input is byte packets; output is queued as MAVLink messages.
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "MavlinkStream", n);
		m_pMavStream = dynamic_cast<MavlinkStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pMavStream);
		NULL_F(m_pBpStreamIn);

		return true;
	}

	bool _APmav_RTCM::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->startThread(getUpdateW, this));
		return m_pTr->startThread(getUpdateR, this);
	}

	bool _APmav_RTCM::check(void)
	{
		NULL_F(m_pMavStream);
		NULL_F(m_pBpStreamIn);

		return this->_ProtocolBase::check();
	}

	void _APmav_RTCM::updateW(void)
	{
		while (m_pT->bRun())
		{
			writeMsg();

			m_pT->sleepT(0);
		}
	}

	void _APmav_RTCM::writeMsg(void)
	{
		IF_(!check());

		// uint64_t tNow = getTns();

		for (size_t i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			uint64_t tLr = pM->m_tLastRecv;

			IF_CONT(pM->m_tLastRecv == 0);
			IF_CONT(pM->m_tLastSent == tLr);

			IF_CONT(!writeMavlink(pM));

			pM->m_tLastSent = tLr;
		}
	}

	bool _APmav_RTCM::writeMavlink(RTCM_MSG *pM)
	{
		NULL_F(pM);
		IF_F(!check());
		IF_F(pM->m_nB == 0 || pM->m_nB > sizeof(pM->m_pB));

		const bool bFragmented = pM->m_nB > GPS_DATA_FRAG_N &&
			pM->m_nB <= 4 * GPS_DATA_FRAG_N;
		uint8_t iFrag = 0;
		for (size_t iB = 0; iB < pM->m_nB;)
		{
			mavlink_gps_rtcm_data_t D{};
			D.flags = m_iSeq << 3;
			if (bFragmented)
				D.flags |= 1 | (iFrag << 1);
			D.len = std::min<size_t>(GPS_DATA_FRAG_N, pM->m_nB - iB);
			memcpy(D.data, pM->m_pB + iB, D.len);
			m_pMavStream->add<MavGpsRTCMdata>(D);
			iB += D.len;
			++iFrag;

			// Payloads over 720 bytes use ordered, unfragmented chunks for the GPS byte stream.
			if (!bFragmented)
				m_iSeq = (m_iSeq + 1) & 0x1F;
		}

		if (bFragmented)
		{
			// A short fragment terminates the sequence unless all four fragments are full.
			if (pM->m_nB % GPS_DATA_FRAG_N == 0 && iFrag < 4)
			{
				mavlink_gps_rtcm_data_t D{};
				D.flags = (m_iSeq << 3) | (iFrag << 1) | 1;
				m_pMavStream->add<MavGpsRTCMdata>(D);
			}
			m_iSeq = (m_iSeq + 1) & 0x1F;
		}

		return true;
	}

	void _APmav_RTCM::updateR(void)
	{
		RTCM_MSG rtcmMsg;
		rtcmMsg.init();

		while (m_pTr->bRun())
		{
			if (readMsg(&rtcmMsg))
			{
				handleMsg(rtcmMsg);
				m_pT->run();
				rtcmMsg.init();
				m_nCMDrecv++;

				continue;
			}

			m_pTr->autoFPS();
		}
	}

	void _APmav_RTCM::handleMsg(const RTCM_MSG &msg)
	{
		uint64_t tNow = getTns();

		for (size_t i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			IF_CONT(pM->m_msgID != msg.m_msgID);

			//			IF_(*pM == msg); // TODO: some msg remains same all the time?

			pM->updateTo(msg);
			pM->m_nRecv++;
			pM->m_tIntSec = ((float)(tNow - pM->m_tLastRecv)) * SEC_NSEC;
			pM->m_tLastRecv = tNow;
			pM->m_tOutRecv.reStart(tNow);
			return;
		}

		RTCM_MSG m;
		m.init();
		m.m_msgID = msg.m_msgID;
		m.updateTo(msg);
		m.m_nRecv++;
		m.m_tLastRecv = tNow;
		m.m_tOutRecv.reStart(tNow);
		m_vMsg.push_back(m);
	}

	void _APmav_RTCM::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_RTCMcast::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iSeq = " + i2str(m_iSeq));
	}

}
