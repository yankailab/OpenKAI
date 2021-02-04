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
    m_pTr = NULL;
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
    DEL(m_pTr);
}

bool _UDP::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("addr",&m_addr);
	pK->v("port", &m_port);
    
    Kiss* pKt = pK->child("threadR");
    IF_F(pKt->empty());
    
    m_pTr = new _Thread();
    if(!m_pTr->init(pKt))
    {
        DEL(m_pTr);
        return false;
    }

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
    NULL_F(m_pT);
    NULL_F(m_pTr);
	IF_F(!m_pT->start(getUpdateW, this));
	return m_pTr->start(getUpdateR, this);
}

void _UDP::updateW(void)
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
			IF_CONT(m_sAddr.sin_addr.s_addr == htonl(INADDR_ANY));

			int nSend = ::sendto(m_socket, pB, nB, 0, (struct sockaddr *) &m_sAddr, m_nSAddr);
			if (nSend == -1)
			{
				LOG_I("sendto error: " + i2str(errno));
				break;
			}
			LOG_I("send: " + i2str(nSend) + " bytes to " + string(inet_ntoa(m_sAddr.sin_addr)) + ", port:" + i2str(ntohs(m_sAddr.sin_port)));
		}

		m_pT->autoFPSto();
	}
}

void _UDP::updateR(void)
{
	while(m_pTr->bRun())
	{
		while(!isOpen())
		{
			::sleep(1);
		}

		uint8_t pB[N_IO_BUF];
		int nR = ::recvfrom(m_socket, pB, N_IO_BUF, 0, (struct sockaddr *) &m_sAddr, &m_nSAddr);
		if (nR <= 0)
		{
			LOG_E("recvfrom error: " + i2str(errno));
			close();
			continue;
		}

		m_fifoR.input(pB,nR);

		LOG_I("Received "+ i2str(nR) +" bytes from ip:" + string(inet_ntoa(m_sAddr.sin_addr)) + ", port:" + i2str(ntohs(m_sAddr.sin_port)));
	}
}

void _UDP::draw(void)
{
	this->_IOBase::draw();
	addMsg("Port:" + i2str(m_port), 1);
}

}
