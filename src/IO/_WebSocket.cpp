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

	bool _WebSocket::init(const json &j)
	{
		IF_F(!this->_IObase::init(j));

		int nPacket = 1024;
		int nPbuffer = 512;
		jVar(j, "nPacket", nPacket);
		jVar(j, "nPbuffer", nPbuffer);

		IF_F(!m_packetW.init(nPbuffer, nPacket));
		IF_F(!m_packetR.init(nPbuffer, nPacket));

		return true;
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
