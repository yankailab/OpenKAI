/*
 * _WebSocket.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_WebSocket.h"

namespace kai
{

	_WebSocket::_WebSocket()
	{
		m_pTr = nullptr;

		m_ioType = io_webSocket;
		m_ioStatus = io_unknown;

		pthread_mutex_init(&m_mutexW, NULL);
	}

	_WebSocket::~_WebSocket()
	{
		pthread_mutex_destroy(&m_mutexW);
		close();

		DEL(m_pTr);
//		m_fifoR.release();
	}

	int _WebSocket::init(void *pKiss)
	{
		CHECK_(this->_IObase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

//		pK->v("fifoIn", &m_fifoIn);

//		IF__(!m_fifoR.init(m_nFIFO), OK_ERR_ALLOCATION);

		Kiss *pKt = pK->child("threadR");
		if (pKt->empty())
		{
			LOG_E("threadR not found");
			return OK_ERR_NOT_FOUND;
		}

		m_pTr = new _Thread();
		CHECK_d_l_(m_pTr->init(pKt), DEL(m_pTr), "threadR init failed");

		return OK_OK;
	}

	int _WebSocket::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	void _WebSocket::updateW(void)
	{
		signal(SIGPIPE, SIG_IGN);

		while (m_pT->bAlive())
		{
			if (!bOpen())
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			uint8_t pB[WS_N_BUF];
			int nB;
			while ((nB = m_packetW.getPacket(pB, WS_N_BUF)) > 0)
			{
				// int nSent = ::write(m_fdW, pB, nB);
				// if (nSent == -1)
				// {
				// 	LOG_E("write error: " + i2str(errno));
				// 	close();
				// 	break;
				// }
			}

			m_pT->autoFPSto();
		}
	}

	void _WebSocket::updateR(void)
	{
		while (m_pTr->bAlive())
		{
			if (!bOpen())
			{
				::sleep(1);
				continue;
			}

			// uint8_t pB[WS_N_BUF];
			// int nR = ::read(m_fdR, pB, WS_N_BUF);
			// if (nR <= 0)
			// {
			// 	close();
			// 	continue;
			// }

			// m_fifoR.input(pB, nR);

			m_pTr->runAll();
		}
	}

	int _WebSocket::read(uint8_t *pBuf, int nB)
	{
		pthread_mutex_lock(&m_mutexW);
		pthread_mutex_unlock(&m_mutexW);

//		return readFrom(m_vClient[0].m_id, pBuf, nB);
	}

	void _WebSocket::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

//		((_Console *)pConsole)->addMsg("nClients: " + i2str(m_vClient.size()), 1);
	}

}
