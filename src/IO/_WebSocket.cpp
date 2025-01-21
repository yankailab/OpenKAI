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
		m_ioType = io_webSocket;
		m_ioStatus = io_unknown;
	}

	_WebSocket::~_WebSocket()
	{
		m_packetR.clear();
		close();
	}

	int _WebSocket::init(void *pKiss)
	{
		CHECK_(this->_IObase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		int nPacket = 1024;
		int nPbuffer = 512;
		pK->v("nPacket", &nPacket);
		pK->v("nPbuffer", &nPbuffer);
		IF__(!m_packetR.init(nPbuffer, nPacket), OK_ERR_ALLOCATION);

		return OK_OK;
	}

	// void _WebSocket::update(void)
	// {
	// 	signal(SIGPIPE, SIG_IGN);

	// 	while (m_pT->bAlive())
	// 	{
	// 		m_pT->autoFPS();

	// 		uint8_t pB[WS_N_BUF];
	// 		int nB;
	// 		while ((nB = m_packetW.getPacket(pB, WS_N_BUF)) > 0)
	// 		{
	// 			int nSent = ::write(m_fdW, pB, nB);
	// 			if (nSent == -1)
	// 			{
	// 				LOG_E("write error: " + i2str(errno));
	// 				close();
	// 				break;
	// 			}
	// 		}
	// 	}
	// }

	int _WebSocket::read(uint8_t *pBuf, int nB)
	{
		NULL__(pBuf, 0);

		return m_packetR.getPacket(pBuf, nB);
	}

	IO_PACKET_FIFO *_WebSocket::getPacketFIFOr(void)
	{
		return &m_packetR;
	}

	void _WebSocket::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);
	}

}
