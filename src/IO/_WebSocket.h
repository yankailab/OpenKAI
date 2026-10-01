/*
 * _webSocket.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__WebSocket_H_
#define OpenKAI_src_IO__WebSocket_H_

#include "_IObase.h"

namespace kai
{
	class _WebSocket : public _IObase
	{
	public:
		_WebSocket();
		virtual ~_WebSocket();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start(void) override;
		virtual void console(void *pConsole);

		BytePacketStream *getBytePacketStreamIn(void);
		BytePacketStream *getBytePacketStreamOut(void);

	protected:
		// Accepted clients own streams when no named DataObjects are configured.
		BytePacketStream m_bpStreamIn;
		BytePacketStream m_bpStreamOut;

	};

}
#endif
