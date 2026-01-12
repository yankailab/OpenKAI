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
	}

	_TCPclient::~_TCPclient()
	{
		close();
	}

	int _TCPclient::init(const json& j)
	{
		CHECK_(this->_IObase::init(j));

		= j.value("addr", &m_strAddr);
		= j.value("port", (int *)&m_port);

		m_bClient = true;

		return true;
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
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _TCPclient::update(void)
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

			uint8_t pB[N_TCP_BUF];
			int nB;
			while ((nB = m_packetW.getPacket(pB, N_TCP_BUF)) > 0)
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

		}
	}

	int _TCPclient::read(uint8_t *pBuf, int nB)
	{
		if (!bOpen())
			return -1;

		int nR = ::recv(m_socket, pBuf, nB, 0);
		if (nR <= 0)
		{
			LOG_E("recv error: " + i2str(errno));
			close();
		}

		return nR;
	}

	void _TCPclient::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

		string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port) + ((m_bClient) ? "; Client" : "; Server");
		((_Console *)pConsole)->addMsg(msg);
	}

}
