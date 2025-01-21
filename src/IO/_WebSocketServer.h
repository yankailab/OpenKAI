/*
 * _WebSocketServer.h
 *
 *  Created on: Nov. 8, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__WebSocketServer_H_
#define OpenKAI_src_IO__WebSocketServer_H_

#include "_WebSocket.h"

namespace kai
{
	struct wsClient
	{
		uint32_t m_id;
		IO_PACKET_FIFO m_packetW;
		uint64_t m_tStamp;

		int init(uint32_t id, int nB, int nP)
		{
			m_id = id;
			m_tStamp = getApproxTbootUs();
			IF_F(!m_packetW.init(nB, nP));
			reset();

			return true;
		}

		void reset(void)
		{
			m_packetW.clear();
		}
	};

	class _WebSocketServer : public _ModuleBase
	{
	public:
		_WebSocketServer();
		virtual ~_WebSocketServer();

		int init(void *pKiss);
		int start(void);
		void console(void *pConsole);

		int nClient(void);

		static void sCbOpen(ws_cli_conn_t client);
		static void sCbClose(ws_cli_conn_t client);
		static void sCbMessage(ws_cli_conn_t client, const unsigned char *msg, uint64_t size, int type);

	private:
		void cbOpen(ws_cli_conn_t client);
		void cbClose(ws_cli_conn_t client);
		void cbMessage(ws_cli_conn_t client, const unsigned char *msg, uint64_t size, int type);

		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_WebSocketServer *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_WebSocketServer *)This)->updateR();
			return NULL;
		}

	protected:
		vector<wsClient> m_vClient;
		string m_host;
		uint16_t m_port;
		uint32_t m_tOutMs;

		_Thread *m_pTr;

	};

}
#endif
