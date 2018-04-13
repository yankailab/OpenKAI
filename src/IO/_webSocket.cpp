/*
 * _peer.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_webSocket.h"

namespace kai
{

_webSocket::_webSocket()
{
	pthread_mutex_init(&m_mutexSend, NULL);
	pthread_mutex_init(&m_mutexRecv, NULL);

	m_strAddr = "";
	m_port = 0;
	m_pBuf = NULL;
	m_nBuf = N_BUF_IO;
}

_webSocket::~_webSocket()
{
	reset();
	pthread_mutex_destroy (&m_mutexSend);
	pthread_mutex_destroy (&m_mutexRecv);
}

bool _webSocket::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("addr", &m_strAddr));
	F_INFO(pK->v("port", (int* )&m_port));

	F_INFO(pK->v("buf", &m_nBuf));
	if (m_nBuf < N_BUF_IO)
		m_nBuf = N_BUF_IO;
	m_pBuf = new uint8_t[m_nBuf];
	NULL_F(m_pBuf);

	return true;
}

void _webSocket::reset(void)
{
	this->_ThreadBase::reset();
	close();

	DEL(m_pBuf);
}

bool _webSocket::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _webSocket::start(void)
{
	IF_T(m_bThreadON);

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

void _webSocket::update(void)
{
	//client mode always trying to connect to the server
	while (m_bThreadON)
	{

		this->autoFPSfrom();

		send();
		recv();

		this->autoFPSto();
	}

}

void _webSocket::send(void)
{
	IF_(m_queSend.empty());

	pthread_mutex_lock(&m_mutexSend);
	int nByte = 0;
	while (!m_queSend.empty() && nByte < m_nBuf)
	{
		m_pBuf[nByte++] = m_queSend.front();
		m_queSend.pop();
	}
	pthread_mutex_unlock(&m_mutexSend);

//	int nSend = ::write(m_webSocket, m_pBuf, nByte);
//	if (nSend == -1)
//	{
//		IF_(errno == EAGAIN);
//		IF_(errno == EWOULDBLOCK);
//		LOG_E("write error: "<<errno);
//		close();
//		return;
//	}
}

void _webSocket::recv(void)
{
//	int nRecv = ::recv(m_webSocket, m_pBuf, m_nBuf, 0);

	pthread_mutex_lock(&m_mutexRecv);
//	for(int i=0; i<nRecv; i++)
//	{
//		m_queRecv.push(m_pBuf[i]);
//	}
	pthread_mutex_unlock(&m_mutexRecv);
}

bool _webSocket::write(uint8_t* pBuf, int nByte)
{
	IF_F(nByte <= 0);
	NULL_F(pBuf);

	pthread_mutex_lock(&m_mutexSend);

	int i;
	for (i = 0; i < nByte; i++)
		m_queSend.push(pBuf[i]);

	pthread_mutex_unlock(&m_mutexSend);

	return true;
}

int _webSocket::read(uint8_t* pBuf, int nByte)
{
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

void _webSocket::close(void)
{
	while (!m_queSend.empty())
		m_queSend.pop();
	while (!m_queRecv.empty())
		m_queRecv.pop();

	LOG_I("Closed");
}

bool _webSocket::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port);
	pWin->addMsg(&msg);

	return true;
}

}
