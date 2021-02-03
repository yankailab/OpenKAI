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

bool _TCPclient::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("addr", &m_strAddr);
	pK->v("port", (int* )&m_port);

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

	int ret = ::connect(m_socket, (struct sockaddr *) &server, sizeof(server));
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
	IF_(m_ioStatus!=io_opened);
	::close(m_socket);
	this->_IOBase::close();
}

bool _TCPclient::start(void)
{
	int retCode;

	if(!m_bThreadON)
	{
		m_bThreadON = true;
		retCode = pthread_create(&m_threadID, 0, getUpdateW, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bThreadON = false;
			return false;
		}
	}

	if(!m_bRThreadON)
	{
		m_bRThreadON = true;
		retCode = pthread_create(&m_rThreadID, 0, getUpdateR, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bRThreadON = false;
			return false;
		}
	}

	return true;
}

void _TCPclient::updateW(void)
{
	while(m_pT->bRun())
	{
		if (!isOpen())
		{
			if (!open())
			{
				m_pT->sleepTime(USEC_1SEC);
				continue;
			}
		}

		m_pT->autoFPSfrom();

		uint8_t pB[N_IO_BUF];
		int nB;
		while((nB = m_fifoW.output(pB, N_IO_BUF)) > 0)
		{
			int nSend = ::send(m_socket, pB, nB, 0);
			if (nSend == -1)
			{
				if(errno == EAGAIN)break;
				if(errno == EWOULDBLOCK)break;
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
	while(m_pTr->bRun())
	{
		if (!isOpen())
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

		m_fifoR.input(pB,nR);
		this->wakeUpLinked();

		LOG_I("received: " + i2str(nR) + " bytes");
	}
}

bool _TCPclient::bComplete(void)
{
	return m_bComplete;
}

void _TCPclient::draw(void)
{
	this->_ModuleBase::draw();

	string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port) + ((m_bClient) ? "; Client" : "; Server");
	addMsg(msg);
}

}
