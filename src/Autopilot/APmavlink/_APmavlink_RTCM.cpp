#include "_APmavlink_RTCM.h"

namespace kai
{

	_APmavlink_RTCM::_APmavlink_RTCM()
	{
		m_pMav = nullptr;
		m_iSeq = 0;
	}

	_APmavlink_RTCM::~_APmavlink_RTCM()
	{
	}

	bool _APmavlink_RTCM::init(const json &j)
	{
		IF_F(!this->_RTCMcast::init(j));

		return true;
	}

	bool _APmavlink_RTCM::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ProtocolBase::link(j, pM));
		string n;

		n = "";
		n = j.value("_Mavlink", "");
		m_pMav = (_Mavlink *)(pM->findModule(n));
		NULL_F(m_pMav);

		return true;
	}

	bool _APmavlink_RTCM::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	bool _APmavlink_RTCM::check(void)
	{
		NULL_F(m_pMav);

		return this->_ProtocolBase::check();
	}

	void _APmavlink_RTCM::updateW(void)
	{
		while (m_pT->bAlive())
		{
			writeMsg();

			m_pT->sleepT(0);
		}
	}

	void _APmavlink_RTCM::writeMsg(void)
	{
		IF_(!check());

		uint64_t tNow = getTbootUs();

		for (int i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			uint64_t tLr = pM->m_tLastRecv;

			IF_CONT(pM->m_tLastRecv == 0);
			IF_CONT(pM->m_tLastSent == tLr);

			IF_CONT(!writeMavlink(pM));

			pM->m_tLastSent = tLr;
		}
	}

	bool _APmavlink_RTCM::writeMavlink(RTCM_MSG *pM)
	{
		IF_F(!check());

		mavlink_gps_rtcm_data_t D;
		D.flags = (pM->m_nB > GPS_DATA_FRAG_N) ? 1 : 0;

		int iB = 0;
		uint8_t iFrag = 0;
		uint8_t iSeq = 0;
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

	void _APmavlink_RTCM::updateR(void)
	{
		RTCM_MSG rtcmMsg;

		while (m_pTr->bAlive())
		{
			IF_CONT(!readMsg(&rtcmMsg));

			handleMsg(rtcmMsg);
			m_pT->run();
			rtcmMsg.init();
			m_nCMDrecv++;
		}
	}

	void _APmavlink_RTCM::handleMsg(const RTCM_MSG &msg)
	{
		uint64_t tNow = getTbootUs();

		for (int i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			IF_CONT(pM->m_msgID != msg.m_msgID);

			//			IF_(*pM == msg); // TODO: some msg remains same all the time?

			pM->updateTo(msg);
			pM->m_nRecv++;
			pM->m_tIntSec = ((float)(tNow - pM->m_tLastRecv)) / USEC_1SEC;
			pM->m_tLastRecv = tNow;
			pM->m_tOutRecv.reStart();
			return;
		}

		RTCM_MSG m;
		m.init();
		m.m_msgID = msg.m_msgID;
		m.updateTo(msg);
		m.m_nRecv++;
		m.m_tLastRecv = tNow;
		m.m_tOutRecv.reStart();
		m_vMsg.push_back(m);
	}

	void _APmavlink_RTCM::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_RTCMcast::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iSeq = " + i2str(m_iSeq));
	}

}
