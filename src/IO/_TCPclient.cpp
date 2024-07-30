/*
 * _TCPclient.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_TCPclient.h"

namespace kai
{

	_TCPclient::_TCPclient()
	{
		m_strAddr = "";
		m_port = 0;
		m_bClient = true;
		m_socket = 0;
		m_ioType = io_tcp;
		m_ioStatus = io_unknown;
		m_bComplete = false;
	}

	_TCPclient::~_TCPclient()
	{
		close();
	}

	int _TCPclient::init(void *pKiss)
	{
		CHECK_(this->_IObase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("addr", &m_strAddr);
		pK->v("port", (int *)&m_port);

		m_bClient = true;

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

	bool _TCPclient::open(void)
	{
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		IF_F(m_socket < 0);

		struct sockaddr_in server;
		server.sin_addr.s_addr = inet_addr(m_strAddr.c_str());
		server.sin_family = AF_INET;
		server.sin_port = htons(m_port);
		LOG_I("connecting");

		int ret = ::connect(m_socket, (struct sockaddr *)&server, sizeof(server));
		if (ret < 0)
		{
			close();
			LOG_E("connect failed");
			return false;
		}

		m_ioStatus = io_opened;
		LOG_I("connected");

		return true;
	}

	void _TCPclient::close(void)
	{
		IF_(m_ioStatus != io_opened);
		::close(m_socket);
		this->_IObase::close();
	}

	int _TCPclient::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	void _TCPclient::updateW(void)
	{
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

			uint8_t pB[N_IO_BUF];
			int nB;
			while ((nB = m_fifoW.output(pB, N_IO_BUF)) > 0)
			{
				int nSend = ::send(m_socket, pB, nB, 0);
				if (nSend == -1)
				{
					if (errno == EAGAIN)
						break;
					if (errno == EWOULDBLOCK)
						break;
					LOG_E("send error: " + i2str(errno));
					close();
					break;
				}

				LOG_I("send: " + i2str(nSend) + " bytes");
			}

			m_pT->autoFPSto();
		}
	}

	void _TCPclient::updateR(void)
	{
		while (m_pTr->bAlive())
		{
			if (!bOpen())
			{
				::sleep(1);
				continue;
			}

			//blocking mode, no FPS control
			uint8_t pB[N_IO_BUF];
			int nR = ::recv(m_socket, pB, N_IO_BUF, 0);
			if (nR <= 0)
			{
				LOG_E("recv error: " + i2str(errno));
				close();
				continue;
			}

			m_fifoR.input(pB, nR);
			m_pTr->runAll();

			LOG_I("received: " + i2str(nR) + " bytes");
		}
	}

	bool _TCPclient::bComplete(void)
	{
		return m_bComplete;
	}

	void _TCPclient::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

		NULL_(m_pTr);
		m_pTr->console(pConsole);

		string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port) + ((m_bClient) ? "; Client" : "; Server");
		((_Console *)pConsole)->addMsg(msg);
	}

}
