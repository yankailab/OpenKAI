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
		m_ioStatus = io_unknown;
	}

	_WebSocket::~_WebSocket()
	{
		stop();
		close();
	}

	bool _WebSocket::loadConfig(void)
	{
		IF_F(!this->_IObase::loadConfig());
		IF_F(!m_bpStreamIn.clear(1024, 512));
		IF_F(!m_bpStreamOut.clear(1024, 512));

		return true;
	}

	bool _WebSocket::saveConfig(bool bExport)
	{
		IF_F(!_IObase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _WebSocket::link(InstanceMgr *pM)
	{
		IF_F(!_IObase::link(pM));
		if (!m_pBpStreamIn)
		{
			m_pBpStreamIn = &m_bpStreamIn;
		}
		if (!m_pBpStreamOut)
		{
			m_pBpStreamOut = &m_bpStreamOut;
		}
		return true;
	}

	bool _WebSocket::start(void)
	{
		// Accepted endpoints are serviced by their server's read and write workers.
		return true;
	}

	BytePacketStream *_WebSocket::getBytePacketStreamIn(void)
	{
		return m_pBpStreamIn;
	}

	BytePacketStream *_WebSocket::getBytePacketStreamOut(void)
	{
		return m_pBpStreamOut;
	}

	void _WebSocket::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);
	}

}
