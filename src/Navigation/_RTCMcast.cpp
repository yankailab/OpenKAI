/*
 * _RTCMcast.cpp
 *
 *  Created on: Jun 3, 2020
 *      Author: yankai
 */

#include "_RTCMcast.h"

namespace kai
{

	_RTCMcast::_RTCMcast()
	{
		m_pIOsend = nullptr;
	}

	_RTCMcast::~_RTCMcast()
	{
	}

	bool _RTCMcast::init(const json &j)
	{
		IF_F(!this->_ProtocolBase::init(j));

		const json &jM = j.at("RTCMmsg");
		IF__(!jM.is_object(), true);

		for (auto it = jM.begin(); it != jM.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			uint64_t ieSendSec = 1;
			jKv(Ji, "ieSendSec", ieSendSec);
			uint64_t tOutSec = 10;
			jKv(Ji, "tOutSec", tOutSec);

			RTCM_MSG m;
			m.init(ieSendSec * USEC_1SEC, tOutSec * USEC_1SEC);
			jKv(Ji, "ID", m.m_msgID);
			jKv(Ji, "bSendOnceOnly", m.m_bSendOnceOnly);

			m_vMsg.push_back(m);
		}

		return true;
	}

	bool _RTCMcast::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ProtocolBase::link(j, pM));

		string n = "";
		jKv(j, "_IObaseSend", n);
		m_pIOsend = (_IObase *)(pM->findModule(n));
		NULL_F(m_pIOsend);

		return true;
	}

	bool _RTCMcast::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	bool _RTCMcast::check(void)
	{
		NULL_F(m_pIOsend);
		IF_F(!m_pIOsend->bOpen());

		return this->_ProtocolBase::check();
	}

	void _RTCMcast::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			sendMsg();
		}
	}

	void _RTCMcast::sendMsg(void)
	{
		IF_(!check());

		uint64_t tNow = getTbootUs();

		for (int i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			uint64_t tLr = pM->m_tLastRecv;

			IF_CONT(pM->m_tLastRecv == 0);

			if (pM->m_tLastSent != tLr)
			{
				IF_CONT(!m_pIOsend->write(pM->m_pB, pM->m_nB));

				pM->m_tLastSent = tLr;
				pM->m_ieSend.reset(tNow);
				continue;
			}

			IF_CONT(pM->m_bSendOnceOnly);

			IF_CONT(pM->m_tOutRecv.bTout(tNow));
			IF_CONT(!pM->m_ieSend.updateT(tNow, false));
			IF_CONT(!m_pIOsend->write(pM->m_pB, pM->m_nB));

			pM->m_tLastSent = tLr;
			pM->m_ieSend.reset(tNow);
		}
	}

	void _RTCMcast::updateR(void)
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

	bool _RTCMcast::readMsg(RTCM_MSG *pMsg)
	{
		IF_F(!check());
		NULL_F(pMsg);

		if (m_nRead == 0)
		{
			m_nRead = m_pIO->read(m_pBuf, RTCM_N_BUF);
			IF_F(m_nRead <= 0);
			m_iRead = 0;
		}

		while (m_iRead < m_nRead)
		{
			bool r = pMsg->input(m_pBuf[m_iRead++]);
			if (m_iRead == m_nRead)
			{
				m_iRead = 0;
				m_nRead = 0;
			}

			IF__(r, true);
		}

		return false;
	}

	void _RTCMcast::handleMsg(const RTCM_MSG &msg)
	{
		uint64_t tNow = getTbootUs();

		for (int i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			IF_CONT(pM->m_msgID != msg.m_msgID);

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

	void _RTCMcast::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ProtocolBase::console(pConsole);

		uint64_t tNow = getTbootUs();

		_Console *pC = (_Console *)pConsole;
		for (int i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			pC->addMsg("msgID: " + i2str(pM->m_msgID) + ", nB=" + i2str(pM->m_nB) + ", nRecv=" + i2str(pM->m_nRecv) + ", tIntSec=" + f2str(pM->m_tIntSec, 2) + ", bTout=" + i2str(pM->m_tOutRecv.bTout(tNow)));
		}
	}

}
