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
	m_socket = -1;
	m_addr = "";
	m_port = DEFAULT_UDP_PORT;
	m_nSAddr = 0;

	m_ioType = io_udp;
	m_ioStatus = io_unknown;
}

_UDP::~_UDP()
{
	close();
}

bool _UDP::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, addr);
	F_INFO(pK->v("port", (int*)&m_port));

	return true;
}

bool _UDP::open(void)
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	IF_F(m_socket < 0);

	m_nSAddr = sizeof(m_sAddr);
	memset((char *) &m_sAddr, 0, m_nSAddr);
	m_sAddr.sin_family = AF_INET;
	m_sAddr.sin_port = htons(m_port);

	if(!m_addr.empty())
	{
		//client mode
		m_sAddr.sin_addr.s_addr = inet_addr(m_addr.c_str());
	}
	else
	{
		//server mode
		m_sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	    IF_F(bind(m_socket , (struct sockaddr*)&m_sAddr, m_nSAddr) == -1);
	}

	m_ioStatus = io_opened;
	return true;
}

void _UDP::close(void)
{
	IF_(m_ioStatus!=io_opened);

	::close(m_socket);
	this->_IOBase::close();
}

bool _UDP::start(void)
{
	int retCode;

	if(!m_bThreadON)
	{
		m_bThreadON = true;
		retCode = pthread_create(&m_threadID, 0, getUpdateThreadW, this);
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
		retCode = pthread_create(&m_rThreadID, 0, getUpdateThreadR, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bRThreadON = false;
			return false;
		}
	}

	return true;
}

void _UDP::updateW(void)
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

		IO_BUF ioB;
		while(toBufW(&ioB))
		{
			int nSend = ::sendto(m_socket, ioB.m_pB, ioB.m_nB, 0, (struct sockaddr *) &m_sAddr, m_nSAddr);
			if (nSend == -1)
			{
				LOG_I("sendto error: " + i2str(errno));
				break;
			}
		}

		this->autoFPSto();
	}
}

void _UDP::updateR(void)
{
	while (m_bRThreadON)
	{
		while(!isOpen())
		{
			::sleep(1);
		}

		IO_BUF ioB;
		ioB.m_nB = ::recvfrom(m_socket, ioB.m_pB, N_IO_BUF, 0, (struct sockaddr *) &m_sAddr, &m_nSAddr);

		if (ioB.m_nB <= 0)
		{
			LOG_E("recvfrom error: " + i2str(errno));
			close();
			continue;
		}

		toQueR(&ioB);

		LOG_I("Received from ip:" + string(inet_ntoa(m_sAddr.sin_addr)) + ", port:" + i2str(ntohs(m_sAddr.sin_port)));
	}
}

bool _UDP::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;

	pWin->tabNext();

	string msg = "Port:" + i2str(m_port);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

}
