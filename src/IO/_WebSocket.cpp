/*
 * _WebSocket.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_WebSocket.h"

namespace kai
{

_WebSocket::_WebSocket()
{
	m_fifo = "/tmp/wspipeout.fifo";
	m_fd = 0;
	m_ioType = io_fifo;
	m_ioStatus = io_unknown;
	m_iMB = 0;
}

_WebSocket::~_WebSocket()
{
	reset();
}

bool _WebSocket::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, fifo);

	m_vClient.clear();
	return true;
}

bool _WebSocket::open(void)
{
	m_fd = ::open(m_fifo.c_str(), O_RDWR);
	IF_F(m_fd < 0);

	m_ioStatus = io_opened;
	return true;
}

void _WebSocket::reset(void)
{
	m_vClient.clear();
	this->_IOBase::reset();
	close();
}

void _WebSocket::close(void)
{
	IF_(m_ioStatus != io_opened);

	::close(m_fd);
	this->_IOBase::close();
}

bool _WebSocket::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _WebSocket::start(void)
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

void _WebSocket::update(void)
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
		decodeMsg();

		this->autoFPSto();
	}
}

bool _WebSocket::writeTo(uint32_t id, uint8_t* pBuf, int nB)
{
	uint8_t pH[12];
	pack_uint32(&pH[0], id);
	pack_uint32(&pH[4], 0x2);
	pack_uint32(&pH[8], nB);
	IF_F(!write(pH,WS_N_HEADER));

	return write(pBuf, nB);
}

int _WebSocket::readFrom(uint32_t id, uint8_t* pBuf, int nB)
{
	if (pBuf == NULL)
		return -1;
	if (nB <= N_IO_BUF)
	{
		LOG_E("nB should be >= " << N_IO_BUF << " bytes");
		return -1;
	}

	WS_CLIENT* pC = findClientById(id);
	if (!pC)
		return -1;
	if (pC->m_qR.empty())
		return 0;

	pthread_mutex_lock(&m_mutexR);
	IO_BUF ioB = pC->m_qR.front();
	pC->m_qR.pop();
	pthread_mutex_unlock(&m_mutexR);

	memcpy(pBuf, ioB.m_pB, ioB.m_nB);
	return ioB.m_nB;
}

void _WebSocket::writeIO(void)
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

void _WebSocket::readIO(void)
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

	if (ioB.m_nB == 0)
	{
		LOG_E("socket is shutdown");
		close();
		return;
	}

	toQueR(&ioB);

//	LOG_I("Received "<< ioB.m_nB <<" bytes from " << inet_ntoa(m_sAddr.sin_addr) << ":" << ntohs(m_sAddr.sin_port));
}

void _WebSocket::decodeMsg(void)
{
	int iR = read(&m_pMB[m_iMB], WS_N_MSG - m_iMB);
	IF_(iR <= 0);

	m_iMB += iR;
	IF_(m_iMB < WS_N_HEADER);

	uint32_t nPayload;
	unpack_uint32(&m_pMB[8], &nPayload);
	uint32_t nMsg = nPayload + WS_N_HEADER;
	IF_(m_iMB < nMsg);

	uint32_t id;
	unpack_uint32(m_pMB, &id);
	WS_CLIENT* pC = findClientById(id);
	if (!pC)
	{
		WS_CLIENT wc;
		wc.init(id);
		m_vClient.push_back(wc);
		pC = &m_vClient[m_vClient.size() - 1];
	}

	IO_BUF ioB;
	int nW = 0;
	while (nW < nPayload)
	{
		ioB.m_nB = nPayload - nW;
		if (ioB.m_nB > N_IO_BUF)
			ioB.m_nB = N_IO_BUF;

		memcpy(ioB.m_pB, &m_pMB[nW + WS_N_HEADER], ioB.m_nB);
		nW += ioB.m_nB;

		pthread_mutex_lock(&m_mutexR);
		pC->m_qR.push(ioB);
		pthread_mutex_unlock(&m_mutexR);
	}

	int nNext = m_iMB - nMsg;
	m_iMB = 0;
	IF_(nNext <= 0);

	for (int i = 0; i < nNext; i++)
	{
		m_pMB[i] = m_pMB[nMsg + i];
	}
	m_iMB = nNext;
}

WS_CLIENT* _WebSocket::findClientById(uint32_t id)
{
	int nClient = m_vClient.size();
	int i;
	for (i = 0; i < nClient; i++)
	{
		IF_CONT(m_vClient[i].m_id != id);
		break;
	}
	IF_N(i >= nClient);

	return &m_vClient[i];
}

bool _WebSocket::draw(void)
{
	IF_F(!this->_IOBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = "Peer IP: ";
	pWin->addMsg(&msg);

	return true;
}

}
