/*
 * _peer.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_socket.h"

namespace kai
{

_socket::_socket()
{
	pthread_mutex_init(&m_mutexSend, NULL);
	pthread_mutex_init(&m_mutexRecv, NULL);

	m_strStatus = "";
	m_strAddr = "";
	m_port = 0;
	m_bClient = true;
	m_bConnected = false;
	m_socket = 0;
	m_pBuf = NULL;
	m_nBuf = N_BUF;
	m_timeoutRecv = TIMEOUT_RECV_USEC;
}

_socket::~_socket()
{
	complete();
	pthread_mutex_destroy (&m_mutexSend);
	pthread_mutex_destroy (&m_mutexRecv);
}

bool _socket::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("addr", &m_strAddr));
	F_INFO(pK->v("port", (int* )&m_port));
	F_INFO(pK->v("timeoutRecv", (int*)&m_timeoutRecv));

	if (m_timeoutRecv < TIMEOUT_RECV_USEC)
		m_timeoutRecv = TIMEOUT_RECV_USEC;

	F_INFO(pK->v("buf", &m_nBuf));
	if (m_nBuf < N_BUF)
		m_nBuf = N_BUF;
	m_pBuf = new uint8_t[m_nBuf];
	NULL_F(m_pBuf);

	m_strStatus = "Initialized";
	m_bClient = true;
	m_bConnected = false;

	return true;
}

bool _socket::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _socket::start(void)
{
	CHECK_T(m_bThreadON);

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _socket::update(void)
{
	//client mode always trying to connect to the server
	while (m_bThreadON)
	{
		if (m_bClient && !m_bConnected)
		{
			if (!connect())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}
		else
		{
			if (!m_bConnected)
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		send();
		recv();

		this->autoFPSto();
	}

}

bool _socket::connect(void)
{
	CHECK_T(m_bConnected);

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_F(m_socket < 0);

	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(m_strAddr.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(m_port);
	m_strStatus = "Connecting";
	LOG_I(m_strStatus);

	int ret = ::connect(m_socket, (struct sockaddr *) &server, sizeof(server));
	if (ret < 0)
	{
		close();
		m_strStatus = "Connection failed";
		LOG_I(m_strStatus);
		return false;
	}

	struct timeval timeout;
	timeout.tv_sec = m_timeoutRecv / USEC_1SEC;
	timeout.tv_usec = m_timeoutRecv % USEC_1SEC;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	m_bConnected = true;
	m_strStatus = "CONNECTED";
	LOG_I(m_strStatus);

	return true;
}

void _socket::send(void)
{
	CHECK_(!m_bConnected);
	CHECK_(m_queSend.empty());

	pthread_mutex_lock(&m_mutexSend);
	int nByte = 0;
	while (!m_queSend.empty() && nByte < m_nBuf)
	{
		m_pBuf[nByte++] = m_queSend.front();
		m_queSend.pop();
	}
	pthread_mutex_unlock(&m_mutexSend);

	int nSend = ::write(m_socket, m_pBuf, nByte);
	if (nSend == -1)
	{
		CHECK_(errno == EAGAIN);
		CHECK_(errno == EWOULDBLOCK);
		LOG_E("write error: "<<errno);
		close();
		return;
	}
}

void _socket::recv(void)
{
	CHECK_(!m_bConnected);

	int nRecv = ::recv(m_socket, m_pBuf, m_nBuf, 0);
	if (nRecv == -1)
	{
		CHECK_(errno == EAGAIN);
		CHECK_(errno == EWOULDBLOCK);
		LOG_E("recv error: "<<errno);
		close();
		return;
	}

	pthread_mutex_lock(&m_mutexRecv);
	for(int i=0;i<nRecv;i++)
	{
		m_queRecv.push(m_pBuf[i]);
	}
	pthread_mutex_unlock(&m_mutexRecv);
}

bool _socket::write(uint8_t* pBuf, int nByte)
{
	CHECK_F(!m_bConnected);
	CHECK_F(nByte <= 0);
	NULL_F(pBuf);

	pthread_mutex_lock(&m_mutexSend);

	int i;
	for (i = 0; i < nByte; i++)
		m_queSend.push(pBuf[i]);

	pthread_mutex_unlock(&m_mutexSend);

	return true;
}

int _socket::read(uint8_t* pBuf, int nByte)
{
	if(!m_bConnected)return -1;
	if(pBuf==NULL)return -1;
	if(nByte<=0)return 0;

	pthread_mutex_lock(&m_mutexRecv);

	int i=0;
	while(!m_queRecv.empty() && i<nByte)
	{
		pBuf[i++] = m_queRecv.front();
		m_queRecv.pop();
	}

	pthread_mutex_unlock(&m_mutexRecv);

	return i;
}

void _socket::close(void)
{
	::close(m_socket);
	m_bConnected = false;

	while (!m_queSend.empty())
		m_queSend.pop();
	while (!m_queRecv.empty())
		m_queRecv.pop();

	LOG_I("Closed");
}

void _socket::complete(void)
{
	close();
	this->_ThreadBase::complete();
}

bool _socket::draw(void)
{
	CHECK_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port) + "; STATUS: "
			+ m_strStatus + ((m_bClient) ? "; Client" : "; Server");
	pWin->addMsg(&msg);

	return true;
}

}
