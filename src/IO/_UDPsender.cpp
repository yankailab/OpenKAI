/*
 * _UDP.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "_UDPsender.h"

namespace kai
{

_UDPsender::_UDPsender()
{
	m_strAddr = "";
	m_port = 0;
	m_socket = 0;
	m_nSAddr = 0;
	m_timeoutRecv = TIMEOUT_RECV_USEC;
}

_UDPsender::~_UDPsender()
{
	complete();
}

bool _UDPsender::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("addr", &m_strAddr));
	F_INFO(pK->v("port", (int* )&m_port));
	F_INFO(pK->v("timeoutRecv", (int*)&m_timeoutRecv));

	return true;
}

bool _UDPsender::link(void)
{
	IF_F(!this->_IOBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _UDPsender::open(void)
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

bool _UDPsender::start(void)
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

void _UDPsender::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		writeIO();
		readIO();

		this->autoFPSto();
	}
}

void _UDPsender::writeIO(void)
{
	IO_BUF ioB;
	toBufW(&ioB);
	IF_(ioB.bEmpty());

	int nSend = ::sendto(m_socket, ioB.m_pB, ioB.m_nB, 0, (struct sockaddr *) &m_sAddr, m_nSAddr);

	if (nSend == -1)
	{
		IF_(errno == EAGAIN);
		IF_(errno == EWOULDBLOCK);
		LOG_E("write error: "<<errno);
		close();
		return;
	}
}

void _UDPsender::readIO(void)
{
	IO_BUF ioB;
	ioB.m_nB = ::recvfrom(m_socket, ioB.m_pB, N_IO_BUF, 0, (struct sockaddr *) &m_sAddr, &m_nSAddr);

	if (ioB.m_nB == -1)
	{
		IF_(errno == EAGAIN);
		IF_(errno == EWOULDBLOCK);
		LOG_E("recv error: "<<errno);
		close();
		return;
	}

	if(ioB.m_nB == 0)
	{
		LOG_E("socket is shutdown by peer");
		close();
		return;
	}

	toQueR(&ioB);
}

void _UDPsender::close(void)
{
	IF_(m_ioStatus!=io_opened);

	::close(m_socket);
	this->_IOBase::close();
}

void _UDPsender::complete(void)
{
	close();
	this->_ThreadBase::complete();
}

bool _UDPsender::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port);
	pWin->addMsg(&msg);

	return true;
}

}
