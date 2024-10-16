/*
 * _UDP.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "_UDP.h"

namespace kai
{

	_UDP::_UDP()
	{
		m_addr = "";
		m_port = 0;
		m_bServer = false;
		m_bW2R = true;
		m_bWbroadcast = 0;

		m_nSAddr = 0;
		m_socket = -1;

		m_ioType = io_udp;
		m_ioStatus = io_unknown;
	}

	_UDP::~_UDP()
	{
		close();
	}

	int _UDP::init(void *pKiss)
	{
		CHECK_(this->_IObase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("addr", &m_addr);
		pK->v("port", &m_port);
		pK->v("bServer", &m_bServer);
		pK->v("bW2R", &m_bW2R);
		pK->v("bWbroadcast", &m_bWbroadcast);

		Kiss *pKt = pK->child("threadR");
		IF__(pKt->empty(), OK_ERR_NOT_FOUND);

		m_pTr = new _Thread();
		CHECK_d_l_(m_pTr->init(pKt), DEL(m_pTr), "thread init failed");

		return OK_OK;
	}

	bool _UDP::open(void)
	{
		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		IF_F(m_socket < 0);

		m_nSAddr = sizeof(m_sAddrW);
		m_addrAny = htonl(INADDR_ANY);

		memset((char *)&m_sAddrW, 0, m_nSAddr);
		m_sAddrW.sin_family = AF_INET;
		m_sAddrW.sin_port = htons(m_port);

		if (m_bServer)
		{
			// server mode
			m_sAddrW.sin_addr.s_addr = htonl(INADDR_ANY);
			IF_F(bind(m_socket, (struct sockaddr *)&m_sAddrW, m_nSAddr) < 0);
		}
		else
		{
			// client mode
			if (m_bWbroadcast)
			{
				IF_F(setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &m_bWbroadcast, sizeof(m_bWbroadcast)) < 0);
				m_addr = "255.255.255.255";
			}

			m_sAddrW.sin_addr.s_addr = inet_addr(m_addr.c_str());
		}

		m_ioStatus = io_opened;
		return true;
	}

	void _UDP::close(void)
	{
		IF_(m_ioStatus != io_opened);

		::close(m_socket);
		this->_IObase::close();
	}

	int _UDP::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	void _UDP::updateW(void)
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
				IF_CONT(m_sAddrW.sin_addr.s_addr == m_addrAny);

				int nSend = ::sendto(m_socket, pB, nB, 0, (struct sockaddr *)&m_sAddrW, m_nSAddr);
				if (nSend < 0)
				{
					LOG_I("sendto error: " + i2str(errno));
					break;
				}
				LOG_I("send: " + i2str(nSend) + " bytes to " + string(inet_ntoa(m_sAddrW.sin_addr)) + ", port:" + i2str(ntohs(m_sAddrW.sin_port)));
			}

			m_pT->autoFPSto();
		}
	}

	void _UDP::updateR(void)
	{
		while (m_pTr->bAlive())
		{
			while (!bOpen())
			{
				::sleep(1);
			}

			uint8_t pB[N_IO_BUF];
			int nR = ::recvfrom(m_socket, pB, N_IO_BUF, 0, (struct sockaddr *)&m_sAddrR, &m_nSAddr);
			if (nR <= 0)
			{
				LOG_E("recvfrom error: " + i2str(errno));
				close();
				continue;
			}

			m_fifoR.input(pB, nR);
			if (m_bW2R)
			{
				m_sAddrW = m_sAddrR;
			}

			m_pTr->runAll();

			LOG_I("Received " + i2str(nR) + " bytes from ip:" + string(inet_ntoa(m_sAddrR.sin_addr)) + ", port:" + i2str(ntohs(m_sAddrR.sin_port)));
		}
	}

	void _UDP::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

		NULL_(m_pTr);
		m_pTr->console(pConsole);

		((_Console *)pConsole)->addMsg("Port:" + i2str(m_port));
	}

}
