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

		m_host = "localhost";
		m_port = 8080;
		m_tOutMs = 1000;
	}

	_WebSocketServer::~_WebSocketServer()
	{
		m_vClient.clear();
	}

	int _WebSocketServer::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("host", &m_host);
		pK->v("port", &m_port);
		pK->v("tOutMs", &m_tOutMs);

		Kiss *pKt = pK->child("thread");
		if (pKt->empty())
		{
			LOG_E("thread not found");
			return OK_ERR_NOT_FOUND;
		}

		m_pTr = new _Thread();
		CHECK_d_l_(m_pTr->init(pKt), DEL(m_pTr), "thread init failed");

		return OK_OK;
	}

	int _WebSocketServer::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	void _WebSocketServer::updateW(void)
	{
		// signal(SIGPIPE, SIG_IGN);

		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
		}
	}

	void _WebSocketServer::updateR(void)
	{
		/*
		 * Main loop, this function never* returns.
		 *
		 * *If the .thread_loop is != 0, a new thread is created
		 * to handle new connections and ws_socket() becomes
		 * non-blocking.
		 */

		/*
		 * Bind host, such as:
		 * localhost -> localhost/127.0.0.1
		 * 0.0.0.0   -> global IPv4
		 * ::        -> global IPv4+IPv6 (Dual stack)
		 */
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

		LOG_I("Connection opened, addr: " + string(cli) + ", port: " + string(port));
	}

	void _WebSocketServer::cbClose(ws_cli_conn_t client)
	{
		char *cli;
		cli = ws_getaddress(client);

		LOG_I("Connection closed, addr: " + string(cli));
	}

	void _WebSocketServer::cbMessage(ws_cli_conn_t client,
									 const unsigned char *msg, uint64_t size, int type)
	{
		char *cli;
		cli = ws_getaddress(client);

		LOG_I("Received message: " + string((char *)msg) + ", size: " + i2str(size) + ", type: " + i2str(type) + ", from: " + string(cli));

		/**
		 * Mimicks the same frame type received and re-send it again
		 *
		 * Please note that we could just use a ws_sendframe_txt()
		 * or ws_sendframe_bin() here, but we're just being safe
		 * and re-sending the very same frame type and content
		 * again.
		 *
		 * Alternative functions:
		 *   ws_sendframe()
		 *   ws_sendframe_txt()
		 *   ws_sendframe_txt_bcast()
		 *   ws_sendframe_bin()
		 *   ws_sendframe_bin_bcast()
		 */
		ws_sendframe_bcast(8080, (char *)msg, size, type);
	}

	void _WebSocketServer::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		((_Console *)pConsole)->addMsg("nClients: " + i2str(m_vClient.size()), 1);
	}

}
