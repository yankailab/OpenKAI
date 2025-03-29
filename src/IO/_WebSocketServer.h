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
		_WebSocket *m_pWS;
		ws_cli_conn_t m_wsConn;
		uint64_t m_tStamp;

		bool init(_WebSocket* pWS)
		{
			NULL_F(pWS);

			m_pWS = pWS;
			m_tStamp = getApproxTbootUs();
			return true;
		}

		void setWS(_WebSocket *pWS)
		{
			m_pWS = pWS;
		}

		_WebSocket *getWS(void)
		{
			return m_pWS;
		}
	};

	enum WSSOCKET_MODE
	{
		wsSocket_bin = 0,
		wsSocket_bin_bcast = 1,
		wsSocket_txt = 2,
		wsSocket_txt_bcast = 3,
	};

	class _WebSocketServer : public _IObase
	{
	public:
		_WebSocketServer();
		virtual ~_WebSocketServer();

		int init(void *pKiss);
		int link(void);
		int start(void);
		void console(void *pConsole);

		// default to client 0
		bool write(uint8_t *pBuf, int nB);
		int read(uint8_t *pBuf, int nB);

		int nClient(void);
		_WebSocket* getClient(int i);

		static void sCbOpen(ws_cli_conn_t client);
		static void sCbClose(ws_cli_conn_t client);
		static void sCbMessage(ws_cli_conn_t client, const unsigned char *msg, uint64_t size, int type);

	private:
		void cbOpen(ws_cli_conn_t client);
		void cbClose(ws_cli_conn_t client);
		void cbMessage(ws_cli_conn_t client, const unsigned char *msg, uint64_t size, int type);

		wsClient* findWSclient(ws_cli_conn_t wsCli);
		int findWSclientIdx(ws_cli_conn_t wsCli);
		wsClient *getWSclient(int i);
		void delWSclient(int i);
//		wsClient *findClient(const string& addr, const string& port);

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
		int m_nClientMax;
		WSSOCKET_MODE m_wsMode;

		string m_host;
		uint16_t m_port;
		uint32_t m_tOutMs;

		_Thread *m_pTr;
	};

}
#endif
