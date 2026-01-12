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

	int _RTCMcast::init(const json& j)
	{
		CHECK_(this->_ProtocolBase::init(j));

		Kiss *pR = pK->child("RTCMmsg");
		IF__(pR->empty(), OK_OK);

		int i = 0;
		while (1)
		{
			Kiss *pM = pR->child(i++);
			if (pM->empty())
				break;

			uint64_t ieSendSec = 1;
			uint64_t tOutSec = 10;
			pM->v("ieSendSec", &ieSendSec);
			pM->v("tOutSec", &tOutSec);

			RTCM_MSG m;
			m.init(ieSendSec * USEC_1SEC, tOutSec * USEC_1SEC);
			pM->v("ID", &m.m_msgID);
			pM->v("bSendOnceOnly", &m.m_bSendOnceOnly);

			m_vMsg.push_back(m);
		}

		return true;
	}

	int _RTCMcast::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ProtocolBase::link(j, pM));
		string n;

		n = "";
		= j.value("_IObaseSend", &n);
		m_pIOsend = (_IObase *)(pM->findModule(n));
		NULL__(m_pIOsend);

		return true;
	}

	int _RTCMcast::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _RTCMcast::check(void)
	{
		NULL__(m_pIOsend);
		IF__(!m_pIOsend->bOpen());

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
