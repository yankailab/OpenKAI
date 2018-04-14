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
	m_fifo = "/tmp/wspipeout.fifo";
	m_fd = 0;
	m_ioType = io_fifo;
}

_webSocket::~_webSocket()
{
	reset();
}

bool _webSocket::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,fifo);

	return true;
}

bool _webSocket::open(void)
{
	m_fd = ::open(m_fifo.c_str(), O_RDWR | O_NONBLOCK);
	IF_F(m_fd < 0);

	m_ioStatus = io_opened;
	return true;
}

void _webSocket::reset(void)
{
	this->_IOBase::reset();
	close();
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
	while (m_bThreadON)
	{
		if (!isOpen())
		{
			if (!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		writeIO();
		readIO();

		this->autoFPSto();
	}
}

void _webSocket::writeIO(void)
{
	IO_BUF ioB;
	toBufW(&ioB);
	IF_(ioB.bEmpty());

	int nSent = ::write(m_fd, ioB.m_pB, ioB.m_nB);

	if (nSent == -1)
	{
		IF_(errno == EAGAIN);
		IF_(errno == EWOULDBLOCK);
		LOG_E("write error: "<<errno);
		close();
		return;
	}
}

void _webSocket::readIO(void)
{
	IO_BUF ioB;
	ioB.m_nB = ::read(m_fd, ioB.m_pB, N_IO_BUF);

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

void _webSocket::close(void)
{
	IF_(m_ioStatus!=io_opened);

	//TODO close fifo
	this->_IOBase::close();
}

bool _webSocket::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = "Peer IP: ";
	pWin->addMsg(&msg);

	return true;
}

}
