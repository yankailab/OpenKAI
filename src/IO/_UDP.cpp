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
		m_ioType = io_udp;
		m_ioStatus = io_unknown;
		m_socket = -1;
	}

	_UDP::~_UDP()
	{
		close();
	}

	bool _UDP::init(const json &j)
	{
		IF_F(!this->_IObase::init(j));

		m_addrRemote = j.value("addrRemote", "");
		m_portRemote = j.value("portRemote", 0);
		m_portLocal = j.value("portLocal", 0);
		m_bW2R = j.value("bW2R", true);
		m_bWbroadcast = j.value("bWbroadcast", 0);

		return true;
	}

	bool _UDP::open(void)
	{
		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		IF_F(m_socket < 0);

		// local socket
		memset(&m_sAddrLocal, 0, sizeof(sockaddr_in));
		m_sAddrLocal.sin_family = AF_INET;
		m_sAddrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
		if (m_portLocal != 0)
		{
			m_sAddrLocal.sin_port = htons(m_portLocal);
			IF_F(bind(m_socket, (struct sockaddr *)&m_sAddrLocal, sizeof(sockaddr_in)) < 0);
		}

		// remote dest
		if (m_bWbroadcast)
		{
			IF_F(setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &m_bWbroadcast, sizeof(m_bWbroadcast)) < 0);
			m_addrRemote = "255.255.255.255";
		}

		memset(&m_sAddrRemote, 0, sizeof(sockaddr_in));
		m_sAddrRemote.sin_family = AF_INET;
		m_sAddrRemote.sin_addr.s_addr = inet_addr(m_addrRemote.c_str());
		m_sAddrRemote.sin_port = htons(m_portRemote);

		m_ioStatus = io_opened;
		return true;
	}

	void _UDP::close(void)
	{
		IF_(m_ioStatus != io_opened);

		::close(m_socket);
		this->_IObase::close();
	}

	bool _UDP::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _UDP::update(void)
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

			m_pT->autoFPS();

			uint8_t pB[N_UDP_BUF];
			int nB;
			while ((nB = m_packetW.getPacket(pB, N_UDP_BUF)) > 0)
			{
				IF_CONT(m_sAddrRemote.sin_addr.s_addr == htonl(INADDR_ANY));

				int nSend = ::sendto(m_socket, pB, nB, 0, (struct sockaddr *)&m_sAddrRemote, sizeof(sockaddr_in));
				if (nSend < 0)
				{
					LOG_I("sendto error: " + i2str(errno));
					break;
				}
				LOG_I("send: " + i2str(nSend) + " bytes to " + string(inet_ntoa(m_sAddrRemote.sin_addr)) + ", port:" + i2str(ntohs(m_sAddrRemote.sin_port)));
			}
		}
	}

	int _UDP::read(uint8_t *pBuf, int nB)
	{
		if (!bOpen())
			return -1;

		sockaddr_in sAddrR;
		unsigned int nSaddr = sizeof(sockaddr_in);
		int nR = ::recvfrom(m_socket, pBuf, nB, 0, (struct sockaddr *)&sAddrR, &nSaddr);
		if (nR <= 0)
		{
			LOG_E("recvfrom error: " + i2str(errno));
			close();
		}
		else if (m_bW2R)
		{
			m_sAddrRemote = sAddrR;
		}

		LOG_I("Received " + i2str(nR) + " bytes from ip:" + string(inet_ntoa(sAddrR.sin_addr)) + ", port:" + i2str(ntohs(sAddrR.sin_port)));

		return nR;
	}

	void _UDP::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("PortLocal:" + i2str(m_portLocal));
		pC->addMsg("PortRemote:" + i2str(m_portRemote));
	}

}
