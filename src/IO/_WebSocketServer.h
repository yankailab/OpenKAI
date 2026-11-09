/*
 * _WebSocketServer.h
 *
 *  Created on: Nov. 8, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__WebSocketServer_H_
#define OpenKAI_src_IO__WebSocketServer_H_

#include "_WebSocket.h"
#include <memory>
#include <mutex>

namespace kai
{
	struct wsClient
	{
		_WebSocket *m_pWS = nullptr;
		// Accepted connections own transient configuration, outside the launch document.
		std::shared_ptr<JsonCfg> m_pJcfg;
		uint64_t m_wsConn = 0;
		uint64_t m_tLastBpStreamIn = 0;

		~wsClient()
		{
			delete m_pWS;
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

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start(void) override;
		void pause(void) override;
		void resume(void) override;
		void stop(void) override;
		virtual void console(void *pConsole);

		int nClient(void);
		_WebSocket* getClient(int i);

	private:
		struct Backend;
		bool sendPacket(uint64_t client, const BYTE_PACKET &bp);
		bool sendClient(uint64_t client, const BYTE_PACKET &bp, bool binary);
		void cbOpen(uint64_t client, const string &addr, const string &port);
		void cbClose(uint64_t client);
		bool cbMessage(uint64_t client, const unsigned char *msg, uint64_t size, int type);

		std::shared_ptr<wsClient> findWSclient(uint64_t wsCli);
		void updateW(void) override;
		void updateR(void) override;

	protected:
		InstanceMgr* m_pM = nullptr;
		vector<std::shared_ptr<wsClient>> m_vClient;
		std::mutex m_clientMutex;
		int m_nClientMax = 128;
		WSSOCKET_MODE m_wsMode = wsSocket_txt_bcast;
		bool m_bReadAllClients = false; // Merge complete client messages into the server input stream.

		string m_host = "localhost";
		uint16_t m_port = 8080;
		uint32_t m_tOutMs = 1000;

		std::mutex m_readMutex;
		bool m_bReadPaused = false;
		bool m_bStopping = true;
		std::mutex m_backendMutex;
		std::shared_ptr<Backend> m_backend;
	};

}
#endif
