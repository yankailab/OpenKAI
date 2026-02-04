/*
 * _WebSocketServer.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_WebSocketServer.h"

namespace kai
{
	static _WebSocketServer *g_pWSserver = NULL;

	_WebSocketServer::_WebSocketServer()
	{
		m_pTr = nullptr;
		g_pWSserver = this;
		m_nClientMax = 128;
		m_wsMode = wsSocket_txt_bcast;

		m_host = "localhost";
		m_port = 8080;
		m_tOutMs = 1000;
	}

	_WebSocketServer::~_WebSocketServer()
	{
		m_vClient.clear();
	}

	bool _WebSocketServer::init(const json &j)
	{
		IF_F(!this->_IObase::init(j));

		jKv(j, "wsMode", m_wsMode);
		jKv(j, "host", m_host);
		jKv(j, "port", m_port);
		jKv(j, "tOutMs", m_tOutMs);
		jKv(j, "nClientMax", m_nClientMax);

		DEL(m_pTr);
		m_pTr = createThread(jK(j, "threadR"), "threadR");
		NULL_F(m_pTr);

		m_ioStatus = io_opened;

		return true;
	}

	bool _WebSocketServer::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_IObase::link(j, pM));

		return true;
	}

	bool _WebSocketServer::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	void _WebSocketServer::updateW(void)
	{
		// signal(SIGPIPE, SIG_IGN);

		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			for (wsClient c : m_vClient)
			{
				_WebSocket *pWS = c.getWS();
				IF_CONT(!pWS);

				IO_PACKET_FIFO *pPw = pWS->getPacketFIFOw();
				uint8_t pB[WS_N_BUF];
				int nB;
				while ((nB = pPw->getPacket(pB, WS_N_BUF)) > 0)
				{
					if (m_wsMode == wsSocket_bin)
					{
						ws_sendframe_bin(c.m_wsConn, (char *)pB, nB);
					}
					else if (m_wsMode == wsSocket_bin_bcast)
					{
						ws_sendframe_bin_bcast(c.m_wsConn, (char *)pB, nB);
					}
					else if (m_wsMode == wsSocket_txt)
					{
						pB[nB] = 0;
						ws_sendframe_txt(c.m_wsConn, (char *)pB);
					}
					else if (m_wsMode == wsSocket_txt_bcast)
					{
						pB[nB] = 0;
						ws_sendframe_txt_bcast(c.m_wsConn, (char *)pB);
					}

					/**
					 * Alternative functions:
					 *	 ws_sendframe_bcast(8080, (char *)msg, size, type);
					 *   ws_sendframe()
					 */
				}
			}
		}
	}

	void _WebSocketServer::updateR(void)
	{
		ws_server ws;
		ws.host = m_host.c_str();
		ws.port = m_port;
		ws.thread_loop = 0;
		ws.timeout_ms = m_tOutMs;
		ws.evs.onopen = &sCbOpen;
		ws.evs.onclose = &sCbClose;
		ws.evs.onmessage = &sCbMessage;

		ws_socket(&ws);
	}

	int _WebSocketServer::nClient(void)
	{
		return m_vClient.size();
	}

	_WebSocket *_WebSocketServer::getClient(int i)
	{
		wsClient *pC = getWSclient(i);
		NULL_N(pC);

		return pC->getWS();
	}

	bool _WebSocketServer::write(uint8_t *pBuf, int nB)
	{
		_WebSocket *pWS = getClient(0);
		NULL_F(pWS);

		pWS->write(pBuf, nB);
		return true;
	}

	int _WebSocketServer::read(uint8_t *pBuf, int nB)
	{
		_WebSocket *pWS = getClient(0);
		NULL__(pWS, 0);

		return pWS->read(pBuf, nB);
	}

	void _WebSocketServer::sCbOpen(ws_cli_conn_t client)
	{
		NULL_(g_pWSserver);
		g_pWSserver->cbOpen(client);
	}

	void _WebSocketServer::sCbClose(ws_cli_conn_t client)
	{
		NULL_(g_pWSserver);
		g_pWSserver->cbClose(client);
	}

	void _WebSocketServer::sCbMessage(ws_cli_conn_t client,
									  const unsigned char *msg, uint64_t size, int type)
	{
		NULL_(g_pWSserver);
		g_pWSserver->cbMessage(client, msg, size, type);
	}

	void _WebSocketServer::cbOpen(ws_cli_conn_t client)
	{
		char *cli, *port;
		cli = ws_getaddress(client);
		port = ws_getport(client);

		IF_(m_vClient.size() >= m_nClientMax);

		json j = json::object();
		j["name"] = this->getName() + ".WS" + i2str(m_vClient.size());
		j["class"] = "_WebSocket";
		j["nPacket"] = 1024;
		j["nPbuffer"] = 512;
		json jT = json::object();
		jT["FPS"] = 1;
		j["thread"] = jT;

		_WebSocket *pWS = new _WebSocket();
		pWS->init(j);
		pWS->setIOstatus(io_opened);

		wsClient c;
		c.init(pWS);
		c.m_wsConn = client;

		m_vClient.push_back(c);

		LOG_I("Connection opened, addr: " + string(cli) + ", port: " + string(port));
	}

	void _WebSocketServer::cbClose(ws_cli_conn_t client)
	{
		char *cli;
		cli = ws_getaddress(client);

		// TODO: add mutex
		int iC = findWSclientIdx(client);
		IF_(iC < 0);

		wsClient *pClient = getWSclient(iC);
		NULL_(pClient);

		_WebSocket *pWS = pClient->getWS();
		NULL_(pWS);

		pWS->setIOstatus(io_closed);
		delete pWS;

		delWSclient(iC);

		LOG_I("Connection closed, addr: " + string(cli));
	}

	void _WebSocketServer::cbMessage(ws_cli_conn_t client,
									 const unsigned char *msg, uint64_t size, int type)
	{
		char *cli;
		cli = ws_getaddress(client);

		wsClient *pC = findWSclient(client);
		NULL_(pC);

		_WebSocket *pWS = pC->getWS();
		NULL_(pWS);

		IO_PACKET_FIFO *pFifo = pWS->getPacketFIFOr();
		NULL_(pFifo);

		pFifo->setPacket((uint8_t *)msg, size);

		LOG_I("Received message: " + string((char *)msg) + ", size: " + i2str(size) + ", type: " + i2str(type) + ", from: " + string(cli));
	}

	wsClient *_WebSocketServer::findWSclient(ws_cli_conn_t wsCli)
	{
		return getWSclient(findWSclientIdx(wsCli));
	}

	int _WebSocketServer::findWSclientIdx(ws_cli_conn_t wsCli)
	{
		for (int i = 0; i < m_vClient.size(); i++)
		{
			wsClient *pWSc = &m_vClient[i];
			IF_CONT(pWSc->m_wsConn != wsCli);

			return i;
		}

		return -1;
	}

	wsClient *_WebSocketServer::getWSclient(int i)
	{
		IF_N(i < 0);
		IF_N(m_vClient.size() <= i);

		return &m_vClient[i];
	}

	void _WebSocketServer::delWSclient(int i)
	{
		IF_(i < 0);
		IF_(m_vClient.size() <= i);

		// TODO: add mutex
		m_vClient.erase(m_vClient.begin() + i);
	}

	// wsClient *_WebSocketServer::findClient(const string &addr, const string &port)
	// {
	// }

	void _WebSocketServer::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

		((_Console *)pConsole)->addMsg("nClients: " + i2str(m_vClient.size()), 1);
	}

}
