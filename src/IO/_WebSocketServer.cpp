/*
 * _WebSocketServer.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_WebSocketServer.h"
#include <shared_mutex>

namespace kai
{
	static _WebSocketServer *g_pWSserver = nullptr;
	static std::shared_mutex g_wsCallbackMutex;
	static uintptr_t g_wsCallbackToken = 0;

	_WebSocketServer::_WebSocketServer()
	{
	}

	_WebSocketServer::~_WebSocketServer()
	{
		stop();
	}

	bool _WebSocketServer::loadConfig(void)
	{
		IF_F(!this->_IObase::loadConfig());
		json &j = *m_pJ;

		jKv(j, "wsMode", m_wsMode);
		jKv(j, "host", m_host);
		jKv(j, "port", m_port);
		jKv(j, "tOutMs", m_tOutMs);
		jKv(j, "nClientMax", m_nClientMax);

		return true;
	}

	bool _WebSocketServer::saveConfig(bool bExport)
	{
		IF_F(!_IObase::saveConfig(false));

		json &j = *m_pJ;
		j["wsMode"] = m_wsMode;
		j["host"] = m_host;
		j["port"] = m_port;
		j["tOutMs"] = m_tOutMs;
		j["nClientMax"] = m_nClientMax;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _WebSocketServer::link(InstanceMgr *pM)
	{
		IF_F(!this->_IObase::link(pM));
		m_pM = pM;
		return true;
	}

	bool _WebSocketServer::start(void)
	{
		if (!m_pT || !m_pTr || !bStopped())
		{
			return false;
		}
		{
			std::lock_guard<std::mutex> lock(m_readMutex);
			m_bStopping = false;
			m_bReadPaused = false;
		}
		{
			std::unique_lock<std::shared_mutex> lock(g_wsCallbackMutex);
			m_callbackToken = ++g_wsCallbackToken;
			g_pWSserver = this;
		}
		m_ioStatus = io_opened;
		if (!_IObase::start())
		{
			stop();
			return false;
		}
		return true;
	}

	void _WebSocketServer::pause(void)
	{
		{
			std::lock_guard<std::mutex> lock(m_readMutex);
			m_bReadPaused = true;
		}
		_IObase::pause();
	}

	void _WebSocketServer::resume(void)
	{
		{
			std::lock_guard<std::mutex> lock(m_readMutex);
			m_bReadPaused = false;
		}
		m_readReady.notify_all();
		_IObase::resume();
	}

	void _WebSocketServer::stop(void)
	{
		{
			std::lock_guard<std::mutex> lock(m_readMutex);
			m_bStopping = true;
		}
		m_readReady.notify_all();
		if (m_pT)
		{
			m_pT->stop();
		}
		{
			// wsServer owns detached client readers. Drain callbacks before teardown.
			std::unique_lock<std::shared_mutex> lock(g_wsCallbackMutex);
			if (g_pWSserver == this)
			{
				g_pWSserver = nullptr;
			}
		}
		// wsServer has no listener shutdown API; _Thread destruction cancels accept.
		DEL(m_pTr);
		_IObase::stop();
		m_ioStatus = io_closed;

		vector<std::shared_ptr<wsClient>> vClient;
		{
			std::lock_guard<std::mutex> lock(m_clientMutex);
			vClient.swap(m_vClient);
		}
		for (const auto &pClient : vClient)
		{
			pClient->m_pWS->setIOstatus(io_closed);
			ws_close_client(pClient->m_wsConn);
		}
	}

	void _WebSocketServer::updateW(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun())
			{
				break;
			}

			vector<std::shared_ptr<wsClient>> vClient;
			{
				std::lock_guard<std::mutex> lock(m_clientMutex);
				vClient = m_vClient;
			}
			if (m_pBpStreamIn && !vClient.empty())
			{
				vector<BYTE_PACKET> vBp;
				m_pBpStreamIn->getPackets(vBp, m_tLastBpStreamIn);
				for (const BYTE_PACKET &bp : vBp)
				{
					if (!m_pT->bRun() || !sendPacket(vClient.front()->m_wsConn, bp))
					{
						break;
					}
					m_tLastBpStreamIn = bp.m_tStamp;
				}
			}

			for (const auto &pClient : vClient)
			{
				_WebSocket *pWS = pClient->m_pWS;
				if (!pWS->bOpen())
				{
					continue;
				}
				BytePacketStream *pStream = pWS->getBytePacketStreamIn();
				if (!pStream)
				{
					continue;
				}

				vector<BYTE_PACKET> vBp;
				pStream->getPackets(vBp, pClient->m_tLastBpStreamIn);
				for (const BYTE_PACKET &bp : vBp)
				{
					if (!m_pT->bRun() || !sendPacket(pClient->m_wsConn, bp))
					{
						break;
					}
					pClient->m_tLastBpStreamIn = bp.m_tStamp;
				}
			}
		}
	}

	bool _WebSocketServer::sendPacket(ws_cli_conn_t client, const BYTE_PACKET &bp)
	{
		const char *pB = reinterpret_cast<const char *>(bp.m_vB.data());
		if (m_wsMode == wsSocket_bin || m_wsMode == wsSocket_txt)
		{
			const int type = m_wsMode == wsSocket_bin ? WS_FR_OP_BIN : WS_FR_OP_TXT;
			return ws_sendframe(client, pB, bp.m_vB.size(), type) >= 0;
		}
		if (m_wsMode != wsSocket_bin_bcast && m_wsMode != wsSocket_txt_bcast)
		{
			return false;
		}

		vector<std::shared_ptr<wsClient>> vClient;
		{
			std::lock_guard<std::mutex> lock(m_clientMutex);
			vClient = m_vClient;
		}
		// The library broadcast holds its global client lock over every send.
		// Individual sends leave receive callbacks independent of slow clients.
		const int type = m_wsMode == wsSocket_bin_bcast ? WS_FR_OP_BIN : WS_FR_OP_TXT;
		bool bSent = false;
		for (const auto &pClient : vClient)
		{
			if (!m_pT->bRun())
			{
				return false;
			}
			if (pClient->m_pWS->bOpen() && ws_sendframe(pClient->m_wsConn, pB, bp.m_vB.size(), type) >= 0)
			{
				bSent = true;
			}
		}
		return bSent;
	}

	void _WebSocketServer::updateR(void)
	{
		ws_server ws{};
		ws.host = m_host.c_str();
		ws.port = m_port;
		ws.thread_loop = 0;
		ws.timeout_ms = m_tOutMs;
		ws.evs.onopen = &sCbOpen;
		ws.evs.onclose = &sCbClose;
		ws.evs.onmessage = &sCbMessage;
		ws.context = reinterpret_cast<void *>(m_callbackToken);

		ws_socket(&ws);
	}

	int _WebSocketServer::nClient(void)
	{
		std::lock_guard<std::mutex> lock(m_clientMutex);
		return m_vClient.size();
	}

	_WebSocket *_WebSocketServer::getClient(int i)
	{
		std::lock_guard<std::mutex> lock(m_clientMutex);
		if (i < 0 || static_cast<size_t>(i) >= m_vClient.size())
		{
			return nullptr;
		}
		return m_vClient[i]->m_pWS;
	}

	void _WebSocketServer::sCbOpen(ws_cli_conn_t client)
	{
		std::shared_lock<std::shared_mutex> lock(g_wsCallbackMutex);
		if (g_pWSserver && ws_get_server_context(client) == reinterpret_cast<void *>(g_pWSserver->m_callbackToken))
		{
			g_pWSserver->cbOpen(client);
		}
	}

	void _WebSocketServer::sCbClose(ws_cli_conn_t client)
	{
		std::shared_lock<std::shared_mutex> lock(g_wsCallbackMutex);
		if (g_pWSserver && ws_get_server_context(client) == reinterpret_cast<void *>(g_pWSserver->m_callbackToken))
		{
			g_pWSserver->cbClose(client);
		}
	}

	void _WebSocketServer::sCbMessage(ws_cli_conn_t client,
									  const unsigned char *msg, uint64_t size, int type)
	{
		std::shared_lock<std::shared_mutex> lock(g_wsCallbackMutex);
		if (g_pWSserver && ws_get_server_context(client) == reinterpret_cast<void *>(g_pWSserver->m_callbackToken))
		{
			g_pWSserver->cbMessage(client, msg, size, type);
		}
	}

	void _WebSocketServer::cbOpen(ws_cli_conn_t client)
	{
		const string addr = ws_getaddress(client);
		const string port = ws_getport(client);
		std::lock_guard<std::mutex> lock(m_clientMutex);
		if (m_nClientMax >= 0 && m_vClient.size() >= static_cast<size_t>(m_nClientMax))
		{
			return;
		}

		json j = json::object();
		j["name"] = this->getName() + ".WS" + i2str(m_vClient.size());
		j["class"] = "_WebSocket";
		j["thread"] = {{"FPS", 1}};

		auto pClient = std::make_shared<wsClient>();
		pClient->m_pJcfg = std::make_shared<JsonCfg>();
		pClient->m_pJcfg->setJson(j);
		pClient->m_pWS = new _WebSocket();
		_WebSocket *pWS = pClient->m_pWS;
		pWS->setName(j["name"].get<string>());
		pWS->setConfig(pClient->m_pJcfg.get(), pClient->m_pJcfg->getJson());
		if (!pWS->loadConfig() || !pWS->link(m_pM))
		{
			LOG_E("Accepted WebSocket configuration failed");
			return;
		}
		pWS->setIOstatus(io_opened);
		pClient->m_wsConn = client;
		m_vClient.push_back(pClient);

		LOG_I("Connection opened, addr: " + addr + ", port: " + port);
	}

	void _WebSocketServer::cbClose(ws_cli_conn_t client)
	{
		const string addr = ws_getaddress(client);
		std::lock_guard<std::mutex> lock(m_clientMutex);
		for (auto it = m_vClient.begin(); it != m_vClient.end(); ++it)
		{
			if ((*it)->m_wsConn == client)
			{
				(*it)->m_pWS->setIOstatus(io_closed);
				m_vClient.erase(it);
				LOG_I("Connection closed, addr: " + addr);
				return;
			}
		}
	}

	void _WebSocketServer::cbMessage(ws_cli_conn_t client,
									 const unsigned char *msg, uint64_t size, int type)
	{
		{
			std::unique_lock<std::mutex> lock(m_readMutex);
			while (m_bReadPaused && !m_bStopping)
			{
				m_readReady.wait(lock);
			}
			if (m_bStopping)
			{
				return;
			}
		}

		std::shared_ptr<wsClient> pClient;
		bool bDefaultClient = false;
		{
			std::lock_guard<std::mutex> lock(m_clientMutex);
			pClient = findWSclient(client);
			if (!pClient)
			{
				return;
			}
			bDefaultClient = pClient == m_vClient.front();
		}

		vector<uint8_t> vB(msg, msg + size);
		BytePacketStream *pStream = pClient->m_pWS->getBytePacketStreamOut();
		if (pStream)
		{
			pStream->addPacket(vB);
		}
		// The server stream retains the former default-client routing.
		if (m_pBpStreamOut && bDefaultClient)
		{
			m_pBpStreamOut->addPacket(vB);
		}

		LOG_I("Received message: " + string(reinterpret_cast<const char *>(msg), size) + ", size: " + i2str(size) + ", type: " + i2str(type) + ", from: " + string(ws_getaddress(client)));
	}

	std::shared_ptr<wsClient> _WebSocketServer::findWSclient(ws_cli_conn_t wsCli)
	{
		for (const auto &pClient : m_vClient)
		{
			if (pClient->m_wsConn == wsCli)
			{
				return pClient;
			}
		}
		return nullptr;
	}

	void _WebSocketServer::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);
		((_Console *)pConsole)->addMsg("nClients: " + i2str(nClient()), 1);
	}

}
