/*
 * _TCPsocket.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_TCPsocket.h"

namespace kai
{

_TCPsocket::_TCPsocket()
{
	m_strAddr = "";
	m_port = 0;
	m_bClient = true;
	m_socket = 0;
	m_timeoutRecv = TIMEOUT_RECV_USEC;
	m_ioType = io_tcp;
	m_ioStatus = io_unknown;
}

_TCPsocket::~_TCPsocket()
{
	reset();
}

bool _TCPsocket::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("addr", &m_strAddr));
	F_INFO(pK->v("port", (int* )&m_port));
	F_INFO(pK->v("timeoutRecv", (int*)&m_timeoutRecv));

	if (m_timeoutRecv < TIMEOUT_RECV_USEC)
		m_timeoutRecv = TIMEOUT_RECV_USEC;

	m_bClient = true;
	return true;
}

bool _TCPsocket::open(void)
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
		LOG_E("connec failed");
		return false;
	}

	struct timeval timeout;
	timeout.tv_sec = m_timeoutRecv / USEC_1SEC;
	timeout.tv_usec = m_timeoutRecv % USEC_1SEC;
	IF_F(setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))<0);

	m_ioStatus = io_opened;
	LOG_I("connected");

	return true;
}

void _TCPsocket::close(void)
{
	IF_(m_ioStatus!=io_opened);
	::close(m_socket);
	this->_IOBase::close();
}

void _TCPsocket::reset(void)
{
	this->_IOBase::reset();
	close();
}

bool _TCPsocket::link(void)
{
	IF_F(!this->_IOBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _TCPsocket::start(void)
{
//	IF_T(m_bThreadON);
//
//	m_bThreadON = true;
//	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
//	if (retCode != 0)
//	{
//		LOG_E(retCode);
//		m_bThreadON = false;
//		return false;
//	}

	return true;
}

void _TCPsocket::update(void)
{
//	while (m_bThreadON)
//	{
//		if (!isOpen())
//		{
//			if (!open())
//			{
//				this->sleepTime(USEC_1SEC);
//				continue;
//			}
//		}
//
//		this->autoFPSfrom();
//
//		writeIO();
//		readIO();
//
//		if(!this->bEmptyW())
//			this->disableSleep(true);
//		else
//			this->disableSleep(false);
//
//		this->autoFPSto();
//	}
}

void _TCPsocket::writeIO(void)
{
	IF_(m_ioStatus != io_opened);

	IO_BUF ioB;
	toBufW(&ioB);
	IF_(ioB.bEmpty());

	int nSent = ::send(m_socket, ioB.m_pB, ioB.m_nB, 0);

	if (nSent == -1)
	{
		IF_(errno == EAGAIN);
		IF_(errno == EWOULDBLOCK);
		LOG_E("send error: "<<errno);
		close();
		return;
	}
}

void _TCPsocket::readIO(void)
{
	IF_(m_ioStatus != io_opened);

	IO_BUF ioB;
	ioB.m_nB = ::recv(m_socket, ioB.m_pB, N_IO_BUF, 0);

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
		LOG_E("socket is shutdown");
		close();
		return;
	}

	toQueR(&ioB);

//	LOG_I("Received "<< ioB.m_nB <<" bytes from " << inet_ntoa(m_sAddr.sin_addr) << ":" << ntohs(m_sAddr.sin_port));
}

bool _TCPsocket::draw(void)
{
	IF_F(!this->_IOBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port) + ((m_bClient) ? "; Client" : "; Server");
	pWin->addMsg(&msg);

	return true;
}

}
