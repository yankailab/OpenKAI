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
		Kiss *pK = (Kiss *)pKiss;

		int nPacket = 1024;
		int nPbuffer = 512;
		if (pK)
		{
			pK->v("nPacket", &nPacket);
			pK->v("nPbuffer", &nPbuffer);
		}

		IF__(!m_packetW.init(nPbuffer, nPacket), OK_ERR_ALLOCATION);
		IF__(!m_packetR.init(nPbuffer, nPacket), OK_ERR_ALLOCATION);

		return OK_OK;
	}

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
