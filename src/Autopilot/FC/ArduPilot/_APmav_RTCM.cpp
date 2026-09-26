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
		if (!_RTCMcast::saveConfig(false))
		{
			return false;
		}

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _APmav_RTCM::link(void)
	{
		IF_F(!this->_ProtocolBase::link());	// Do not use _RTCM::link as we send to _Mavlink thus the _IObaseSend is not needed
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_Mavlink", n);
		m_pMav = (_Mavlink *)(m_pM->findModule(n));
		NULL_F(m_pMav);

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
		NULL_F(m_pMav);

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
		IF_F(!check());

		mavlink_gps_rtcm_data_t D;
		D.flags = (pM->m_nB > GPS_DATA_FRAG_N) ? 1 : 0;

		int iB = 0;
		uint8_t iFrag = 0;
		// uint8_t iSeq = 0;
		while (iB < pM->m_nB)
		{
			int nB = pM->m_nB - iB;
			if (nB > GPS_DATA_FRAG_N)
				nB = GPS_DATA_FRAG_N;

			D.flags &= 0x01;
			D.flags |= ((iFrag++) & 0x03) << 1;
			D.flags |= ((m_iSeq) & 0x1F) << 3;
			D.len = nB;
			memcpy(D.data, &pM->m_pB[iB], nB);
			iB += nB;

			m_pMav->gpsRTCMdata(D);
		}

		m_iSeq = (m_iSeq + 1) & 0x1F;

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
