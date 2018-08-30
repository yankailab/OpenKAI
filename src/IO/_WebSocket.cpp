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
	m_fifoW = "/tmp/wspipein.fifo";
	m_fifoR = "/tmp/wspipeout.fifo";
	m_fdW = 0;
	m_fdR = 0;
	m_ioType = io_webSocket;
	m_ioStatus = io_unknown;

	resetDecodeMsg();
	pthread_mutex_init(&m_mutexCR, NULL);

}

_WebSocket::~_WebSocket()
{
	m_vClient.clear();
	close();
	pthread_mutex_destroy(&m_mutexCR);
}

bool _WebSocket::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	IF_Fl(!m_bStream, "_WebSocket only supports m_bStream = 1");

	KISSm(pK, fifoW);
	KISSm(pK, fifoR);

	m_vClient.clear();
	return true;
}

bool _WebSocket::open(void)
{
	m_fdW = ::open(m_fifoW.c_str(), O_WRONLY);
	IF_Fl(m_fdW < 0, "Cannot open: " + m_fifoW);

	m_fdR = ::open(m_fifoR.c_str(), O_RDWR);
	IF_Fl(m_fdR < 0, "Cannot open: " + m_fifoR);

	m_ioStatus = io_opened;
	return true;
}

void _WebSocket::close(void)
{
	IF_(m_ioStatus != io_opened);

	::close(m_fdW);
	::close(m_fdR);
	m_fdW = 0;
	m_fdR = 0;
	this->_IOBase::close();
}

bool _WebSocket::start(void)
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

void _WebSocket::updateW(void)
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
			int nSent = ::write(m_fdW, ioB.m_pB, ioB.m_nB);
			if (nSent == -1)
			{
				LOG_E("write error: " + i2str(errno));
				close();
				break;
			}
		}

		this->autoFPSto();
	}
}

void _WebSocket::updateR(void)
{
	while (m_bRThreadON)
	{
		if (!isOpen())
		{
			::sleep(1);
			continue;
		}

		//blocking mode, no FPS control
		IO_BUF ioB;
		ioB.m_nB = ::read(m_fdR, ioB.m_pB, N_IO_BUF);
		if (ioB.m_nB <= 0)
		{
			close();
			continue;
		}

		toQueR(&ioB);
		decodeMsg();
	}
}

bool _WebSocket::write(uint8_t* pBuf, int nB, uint32_t mode)
{
	IF_F(m_vClient.empty());
	return writeTo(m_vClient[0].m_id, pBuf, nB, mode);
}

int _WebSocket::read(uint8_t* pBuf, int nB)
{
	if(m_vClient.empty())return 0;
	return readFrom(m_vClient[0].m_id, pBuf, nB);
}

bool _WebSocket::writeTo(uint32_t id, uint8_t* pBuf, int nB, uint32_t mode)
{
	IF_F(m_ioStatus != io_opened);
	NULL_F(pBuf);
	IF_F(nB <= 0);

	IO_BUF ioB;
	ioB.m_nB = WS_N_HEADER;
	pack_uint32(&ioB.m_pB[0], id);
	pack_uint32(&ioB.m_pB[4], mode);
	pack_uint32(&ioB.m_pB[8], nB);

	pthread_mutex_lock(&m_mutexW);

	//put header
	m_queW.push(ioB);

	//put payload
	int nW = 0;
	while (nW < nB)
	{
		ioB.m_nB = nB - nW;
		if(ioB.m_nB > N_IO_BUF)
			ioB.m_nB = N_IO_BUF;

		memcpy(ioB.m_pB, &pBuf[nW], ioB.m_nB);
		nW += ioB.m_nB;

		m_queW.push(ioB);
	}

	pthread_mutex_unlock(&m_mutexW);

	return true;
}

int _WebSocket::readFrom(uint32_t id, uint8_t* pBuf, int nB)
{
	if (!pBuf)return -1;
	if (nB < N_IO_BUF)return -1;

	WS_CLIENT* pC = findClientById(id);
	if (!pC)return -1;
	if (pC->m_qR.empty())return 0;

	pthread_mutex_lock(&m_mutexCR);
	IO_BUF ioB = pC->m_qR.front();
	pC->m_qR.pop();
	pthread_mutex_unlock(&m_mutexCR);

	memcpy(pBuf, ioB.m_pB, ioB.m_nB);
	return ioB.m_nB;
}

int _WebSocket::readFrameBuf(uint8_t* pBuf, int nB)
{
	return this->_IOBase::read(pBuf, nB);
}

void _WebSocket::decodeMsg(void)
{
	int i;

	//move data to front
	if(m_iB>0)
	{
		if(m_nB > m_iB)
		{
			for(i=0; m_iB<m_nB; i++,m_iB++)
			{
				m_pMB[i] = m_pMB[m_iB];
			}
			m_nB = i;
		}
		else
		{
			m_nB = 0;
		}
		m_iB = 0;
	}

	int iR = readFrameBuf(&m_pMB[m_nB], N_IO_BUF);
	if(iR > 0)m_nB += iR;
	IF_(m_nB <= 0);

	//decode new msg
	if(m_iMsg >= m_nMsg)
	{
		IF_(m_nB - m_iB < WS_N_HEADER);

		//decode header
		uint32_t id = unpack_uint32(&m_pMB[m_iB]);
		m_pC = findClientById(id);
		if (!m_pC)
		{
			WS_CLIENT wc;
			wc.init(id);
			m_vClient.push_back(wc);
			m_pC = &m_vClient[m_vClient.size() - 1];

			LOG_I("Created new client id: " + i2str(id));
		}

		//decode payload
		m_nMsg = WS_N_HEADER + unpack_uint32(&m_pMB[m_iB+8]);
		m_iMsg = WS_N_HEADER;
		m_iB += WS_N_HEADER;

		if(m_nMsg >= WS_N_MSG)
		{
			resetDecodeMsg();
			return;
		}

		LOG_I("Received from: " + i2str(id) + ", size: " + i2str(m_nMsg));
	}

	//payload decoding
	while(m_iMsg < m_nMsg)
	{
		IF_(m_iB >= m_nB);

		IO_BUF ioB;
		ioB.m_nB = m_nMsg - m_iMsg;
		if(ioB.m_nB > (m_nB - m_iB))ioB.m_nB = (m_nB - m_iB);
		if(ioB.m_nB > N_IO_BUF)ioB.m_nB = N_IO_BUF;

		memcpy(ioB.m_pB, &m_pMB[m_iB], ioB.m_nB);
		pthread_mutex_lock(&m_mutexCR);
		m_pC->m_qR.push(ioB);
		pthread_mutex_unlock(&m_mutexCR);

		m_iB += ioB.m_nB;
		m_iMsg += ioB.m_nB;
	}
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

void _WebSocket::resetDecodeMsg(void)
{
	m_iMsg = 0;
	m_nMsg = 0;
	m_nB = 0;
	m_iB = 0;
	m_pC = NULL;

	if(m_fdR!=0)
	{
		::tcflush(m_fdR, TCIFLUSH);
	}
}

bool _WebSocket::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	pWin->tabNext();

	string msg = "nClients: " + i2str(m_vClient.size());
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

}
