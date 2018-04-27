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

	m_addrW = "";
	m_portW = DEFAULT_UDP_PORT;
	m_nSAddrW = 0;

	m_addrR = "";
	m_portR = DEFAULT_UDP_PORT;
	m_nSAddrR = 0;

	m_ioType = io_udp;
	m_ioStatus = io_unknown;
}

_UDP::~_UDP()
{
	reset();
}

bool _UDP::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, addrW);
	F_INFO(pK->v("portW", (int*)&m_portW));

	KISSm(pK, addrR);
	F_INFO(pK->v("portR", (int*)&m_portR));

	return true;
}

void _UDP::reset(void)
{
	this->_IOBase::reset();
	close();
}

bool _UDP::link(void)
{
	IF_F(!this->_IOBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _UDP::open(void)
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	IF_F(m_socket < 0);

	m_nSAddrW = sizeof(m_sAddrW);
	m_nSAddrR = sizeof(m_sAddrR);

	memset((char *) &m_sAddrR, 0, m_nSAddrR);
	m_sAddrR.sin_family = AF_INET;
	m_sAddrR.sin_port = htons(m_portR);

	if(!m_addrW.empty())
	{
		m_sAddrR.sin_addr.s_addr = inet_addr(m_addrW.c_str());
		m_sAddrR.sin_port = htons(m_portW);
	}
	else
	{
		m_sAddrR.sin_addr.s_addr = htonl(INADDR_ANY);
	    IF_F(bind(m_socket , (struct sockaddr*)&m_sAddrR, m_nSAddrR) == -1);
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

	if(!m_pThreadW->m_bThreadON)
	{
		m_pThreadW->m_bThreadON = true;
		retCode = pthread_create(&m_pThreadW->m_threadID, 0, getUpdateThreadW, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_pThreadW->m_bThreadON = false;
			return false;
		}
	}

	if(!m_pThreadR->m_bThreadON)
	{
		m_pThreadR->m_bThreadON = true;
		retCode = pthread_create(&m_pThreadR->m_threadID, 0, getUpdateThreadR, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_pThreadR->m_bThreadON = false;
			return false;
		}
	}

	return true;
}

void _UDP::updateW(void)
{
	while (m_pThreadW->m_bThreadON)
	{
		if (!isOpen())
		{
			if (!open())
			{
				m_pThreadW->sleepTime(USEC_1SEC);
				continue;
			}
		}

		m_pThreadW->autoFPSfrom();

		IO_BUF ioB;
		while(1)
		{
			toBufW(&ioB);
			if(ioB.bEmpty())break;

			int nSend = ::sendto(m_socket, ioB.m_pB, ioB.m_nB, 0, (struct sockaddr *) &m_sAddrR, m_nSAddrR);
			if (nSend == -1)
			{
				LOG_I("sendto error: "<<errno);
			}
		}

		m_pThreadW->autoFPSto();
	}
}

void _UDP::updateR(void)
{
	while (m_pThreadR->m_bThreadON)
	{
		if (!isOpen())
		{
			m_pThreadR->sleepTime(USEC_1SEC);
			continue;
		}

		IO_BUF ioB;
		ioB.m_nB = ::recvfrom(m_socket, ioB.m_pB, N_IO_BUF, 0, (struct sockaddr *) &m_sAddrR, &m_nSAddrR);

		if (ioB.m_nB == -1)
		{
			LOG_E("recv error: "<<errno);
			close();
			continue;
		}

		if(ioB.m_nB == 0)
		{
			LOG_E("socket is shutdown by peer");
			close();
			continue;
		}

		toQueR(&ioB);

		LOG_I("Received from ip:" << inet_ntoa(m_sAddrW.sin_addr) << ", port:" << ntohs(m_sAddrW.sin_port));
	}
}

bool _UDP::draw(void)
{
	IF_F(!this->_IOBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	pWin->tabNext();

	string msg = "Port:" + i2str(m_portW);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

}
