/*
 * _UDPserver.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "_UDPserver.h"

namespace kai
{

_UDPserver::_UDPserver()
{
	m_strAddr = "";
	m_port = 0;
	m_socket = 0;
	m_nSAddr = 0;
	m_timeoutRecv = TIMEOUT_RECV_USEC;
}

_UDPserver::~_UDPserver()
{
	complete();
}

bool _UDPserver::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("addr", &m_strAddr));
	F_INFO(pK->v("port", (int* )&m_port));
	F_INFO(pK->v("timeoutRecv", (int*)&m_timeoutRecv));

	return true;
}

bool _UDPserver::link(void)
{
	IF_F(!this->_IOBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _UDPserver::open(void)
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	IF_F(m_socket < 0);

	m_nSAddr = sizeof(m_sAddr);
    memset((char *) &m_sAddr, 0, m_nSAddr);
	m_sAddr.sin_addr.s_addr = inet_addr(m_strAddr.c_str());
	m_sAddr.sin_family = AF_INET;
	m_sAddr.sin_port = htons(m_port);

	struct timeval timeout;
	timeout.tv_sec = m_timeoutRecv / USEC_1SEC;
	timeout.tv_usec = m_timeoutRecv % USEC_1SEC;
	IF_F(setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))<0);

	m_ioStatus = io_opened;
	return true;
}

bool _UDPserver::start(void)
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

void _UDPserver::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		writeIO();
		readIO();

		this->autoFPSto();
	}
}

void _UDPserver::writeIO(void)
{
	int nB = m_ioW.que2buf();
	IF_(nB <= 0);

	int nSend = ::sendto(m_socket, m_ioW.m_pBuf, nB, 0, (struct sockaddr *) &m_sAddr, m_nSAddr);

	if (nSend == -1)
	{
		IF_(errno == EAGAIN);
		IF_(errno == EWOULDBLOCK);
		LOG_E("write error: "<<errno);
		close();
		return;
	}
}

void _UDPserver::readIO(void)
{
	int nRecv = ::recvfrom(m_socket, m_ioR.m_pBuf, m_ioR.m_nBuf, 0, (struct sockaddr *) &m_sAddr, &m_nSAddr);

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

	m_ioR.buf2que(nRecv);
}

void _UDPserver::close(void)
{
	IF_(m_ioStatus!=io_opened);

	::close(m_socket);
	this->_IOBase::close();
}

void _UDPserver::complete(void)
{
	close();
	this->_ThreadBase::complete();
}

bool _UDPserver::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port);
	pWin->addMsg(&msg);

	return true;
}

}
