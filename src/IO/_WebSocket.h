/*
 * _webSocket.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__WebSocket_H_
#define OpenKAI_src_IO__WebSocket_H_

#include <wsserver/ws.h>
#include "_IObase.h"

#define WS_N_BUF 512

namespace kai
{
	class _WebSocket : public _IObase
	{
	public:
		_WebSocket();
		virtual ~_WebSocket();

		int init(void *pKiss);
		void console(void *pConsole);

		int read(uint8_t *pBuf, int nB);
		IO_PACKET_FIFO* getPacketFIFOr(void);

	protected:
		IO_PACKET_FIFO m_packetR;

	};

}
#endif
