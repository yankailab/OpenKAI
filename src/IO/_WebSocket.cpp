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

		m_fifoIn = "/tmp/wspipein.fifo";
		m_fifoOut = "/tmp/wspipeout.fifo";
		m_fdW = 0;
		m_fdR = 0;
		m_ioType = io_webSocket;
		m_ioStatus = io_unknown;

		resetDecodeMsg();
		pthread_mutex_init(&m_mutexW, NULL);
	}

	_WebSocket::~_WebSocket()
	{
		pthread_mutex_destroy(&m_mutexW);
		m_vClient.clear();
		close();

		DEL(m_pTr);
		m_fifoR.release();
	}

	int _WebSocket::init(void *pKiss)
	{
		CHECK_(this->_IObase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("fifoIn", &m_fifoIn);
		pK->v("fifoOut", &m_fifoOut);
		m_vClient.clear();

		IF__(!m_fifoR.init(m_nFIFO), OK_ERR_ALLOCATION);

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

	bool _WebSocket::open(void)
	{
		m_fdW = ::open(m_fifoIn.c_str(), O_WRONLY);
		if (m_fdW < 0)
		{
			LOG_E("Cannot open: " + m_fifoIn);
			return false;
		}

		m_fdR = ::open(m_fifoOut.c_str(), O_RDWR);
		if (m_fdR < 0)
		{
			LOG_E("Cannot open: " + m_fifoOut);
			return false;
		}

		m_ioStatus = io_opened;
		return true;
	}

	void _WebSocket::close(void)
	{
		IF_(m_ioStatus != io_opened);

		::close(m_fdW);
		::close(m_fdR);
		m_fdW = 0;
		m_fdR = 0;

		m_fifoR.clear();
		this->_IObase::close();
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
			while ((nB = m_fifoW.output(pB, WS_N_BUF)) > 0)
			{
				int nSent = ::write(m_fdW, pB, nB);
				if (nSent == -1)
				{
					LOG_E("write error: " + i2str(errno));
					close();
					break;
				}
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

			uint8_t pB[WS_N_BUF];
			int nR = ::read(m_fdR, pB, WS_N_BUF);
			if (nR <= 0)
			{
				close();
				continue;
			}

			m_fifoR.input(pB, nR);
			decodeMsg();
			m_pTr->runAll();
		}
	}

	int _WebSocket::nClient(void)
	{
		return m_vClient.size();
	}

	bool _WebSocket::write(uint8_t *pBuf, int nB)
	{
		return write(pBuf, nB, WS_MODE_TXT);
	}

	int _WebSocket::read(uint8_t *pBuf, int nB)
	{
		IF__(m_vClient.empty(), 0);

		return readFrom(m_vClient[0].m_id, pBuf, nB);
	}

	bool _WebSocket::write(uint8_t *pBuf, int nB, uint32_t mode)
	{
		//		IF_F(m_vClient.empty());

		return writeTo(0, pBuf, nB, mode);
	}

	bool _WebSocket::writeTo(uint32_t id, uint8_t *pBuf, int nB, uint32_t mode)
	{
		IF_F(m_ioStatus != io_opened);
		NULL_F(pBuf);
		IF_F(nB <= 0);

		uint8_t pHeader[WS_N_HEADER];
		pack_uint32(&pHeader[0], id);
		pack_uint32(&pHeader[4], mode);
		pack_uint32(&pHeader[8], nB);

		pthread_mutex_lock(&m_mutexW);
		this->_IObase::write(pHeader, WS_N_HEADER);
		this->_IObase::write(pBuf, nB);
		pthread_mutex_unlock(&m_mutexW);

		return true;
	}

	int _WebSocket::readFrom(uint32_t id, uint8_t *pBuf, int nB)
	{
		WS_CLIENT *pC = findClientById(id);
		NULL__(pC, -1);

		return pC->m_fifo.output(pBuf, nB);
	}

	void _WebSocket::decodeMsg(void)
	{
		static uint8_t pMB[WS_N_BUF];
		int i;

		// move data to front
		if (m_iB > 0)
		{
			if (m_nB > m_iB)
			{
				for (i = 0; m_iB < m_nB; i++, m_iB++)
				{
					pMB[i] = pMB[m_iB];
				}
				m_nB = i;
			}
			else
			{
				m_nB = 0;
			}
			m_iB = 0;
		}

		int iR = this->_IObase::read(&pMB[m_nB], WS_N_BUF);
		if (iR > 0)
			m_nB += iR;
		IF_(m_nB <= 0);

		// decode new msg
		if (m_iMsg >= m_nMsg)
		{
			IF_(m_nB - m_iB < WS_N_HEADER);

			// decode header
			uint32_t id = unpack_uint32(&pMB[m_iB]);
			m_pC = findClientById(id);
			if (!m_pC)
			{
				WS_CLIENT wc;
				wc.init(id, m_nFIFO);
				m_vClient.push_back(wc);
				m_pC = &m_vClient[m_vClient.size() - 1];

				LOG_I("Created new client id: " + i2str(id));
			}

			// decode payload
			m_nMsg = WS_N_HEADER + unpack_uint32(&pMB[m_iB + 8]);
			m_iMsg = WS_N_HEADER;
			m_iB += WS_N_HEADER;

			if (m_nMsg >= WS_N_MSG)
			{
				resetDecodeMsg();
				return;
			}

			LOG_I("Received from: " + i2str(id) + ", size: " + i2str(m_nMsg));
		}

		// payload decoding
		while (m_iMsg < m_nMsg)
		{
			IF_(m_iB >= m_nB);
			int nB = m_nMsg - m_iMsg;
			if (nB > (m_nB - m_iB))
				nB = (m_nB - m_iB);

			m_pC->m_fifo.input(&pMB[m_iB], nB);

			m_iB += nB;
			m_iMsg += nB;
		}
	}

	void _WebSocket::resetDecodeMsg(void)
	{
		m_iMsg = 0;
		m_nMsg = 0;
		m_nB = 0;
		m_iB = 0;
		m_pC = NULL;

		if (m_fdR != 0)
		{
			::tcflush(m_fdR, TCIFLUSH);
		}
	}

	WS_CLIENT *_WebSocket::findClientById(uint32_t id)
	{
		int nClient = m_vClient.size();
		int i;
		for (i = 0; i < nClient; i++)
		{
			IF_CONT(m_vClient[i].m_id != id);
			break;
		}
		IF__(i >= nClient, nullptr);

		return &m_vClient[i];
	}

	void _WebSocket::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

		((_Console *)pConsole)->addMsg("nClients: " + i2str(m_vClient.size()), 1);
	}

}
