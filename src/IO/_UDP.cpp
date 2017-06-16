/*
 * _UDPsocket.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "_UDP.h"

namespace kai
{

_UDP::_UDP()
{
	m_strAddr = "";
	m_port = 0;
	m_socket = 0;
	m_nSAddr = 0;
}

_UDP::~_UDP()
{
	complete();
}

bool _UDP::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("addr", &m_strAddr));
	F_INFO(pK->v("port", (int* )&m_port));

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	IF_F(m_socket < 0);

	m_nSAddr = sizeof(m_sAddr);
    memset((char *) &m_sAddr, 0, m_nSAddr);
	m_sAddr.sin_addr.s_addr = inet_addr(m_strAddr.c_str());
	m_sAddr.sin_family = AF_INET;
	m_sAddr.sin_port = htons(m_port);

	return true;
}

bool _UDP::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _UDP::start(void)
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

void _UDP::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		writeIO();
		readIO();

		this->autoFPSto();
	}
}

void _UDP::writeIO(void)
{
	int nB = this->_IOBase::writeIO();
	IF_(nB <= 0);

	int nSend = ::sendto(m_socket, m_pBufIO, nB, 0, (struct sockaddr *) &m_sAddr, m_nSAddr);

	//TODO
	if (nSend == -1)
	{
		IF_(errno == EAGAIN);
		IF_(errno == EWOULDBLOCK);
		LOG_E("write error: "<<errno);
		close();
		return;
	}
}

void _UDP::readIO(void)
{
	int nRecv = ::recvfrom(m_socket, m_pBufIO, m_nBufIO, 0, (struct sockaddr *) &m_sAddr, &m_nSAddr);

	//TODO
	if (nRecv == -1)
	{
		IF_(errno == EAGAIN);
		IF_(errno == EWOULDBLOCK);
		LOG_E("recv error: "<<errno);
		close();
		return;
	}

	if(nRecv == 0)
	{
		LOG_E("socket is shutdown by peer");
		close();
		return;
	}

	this->_IOBase::readIO(nRecv);
}

void _UDP::close(void)
{
	::close(m_socket);
	this->_IOBase::close();

	LOG_I("Closed");
}

void _UDP::complete(void)
{
	close();
	this->_ThreadBase::complete();
}

bool _UDP::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port);
	pWin->addMsg(&msg);

	return true;
}

}
