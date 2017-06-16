/*
 * _webSocketServer.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_webSocketServer.h"

namespace kai
{

_webSocketServer::_webSocketServer()
{
	m_socket = 0;
	m_listenPort = 8888;
	m_nListen = N_LISTEN;
	m_nSocket = N_SOCKET;
	m_strStatus = "";
	m_lSocket.clear();

}

_webSocketServer::~_webSocketServer()
{
	complete();
}

bool _webSocketServer::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("listenPort", (int* )&m_listenPort));
	F_INFO(pK->v("nListen", &m_nListen));
	F_INFO(pK->v("nSocket", &m_nSocket));

	return true;
}

bool _webSocketServer::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _webSocketServer::start(void)
{
	IF_T(m_bThreadON);

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _webSocketServer::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(!handler())
		{
			this->sleepTime(USEC_1SEC);
		}

		this->autoFPSto();
	}

}

bool _webSocketServer::handler(void)
{
	//Create socket
	m_strStatus = "Creating socket";
	LOG_I(m_strStatus);
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	IF_F(m_socket == -1);

	//Prepare the sockaddr_in structure
	m_webSocketServerAddr.sin_family = AF_INET;
	m_webSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
	m_webSocketServerAddr.sin_port = htons(m_listenPort);

	int yes = 1;
	setsockopt(m_socket,
	   SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

	//Bind
	m_strStatus = "Binding";
	LOG_I(m_strStatus);
	if (bind(m_socket, (struct sockaddr *) &m_webSocketServerAddr, sizeof(m_webSocketServerAddr)) < 0)
	{
		m_strStatus = "Binding failed";
		LOG_I(m_strStatus);
		close(m_socket);
		return false;
	}

	//Listen
	m_strStatus = "Listening";
	LOG_I(m_strStatus);
	listen(m_socket, m_nListen);

	//Accept incoming connection
	m_strStatus = "Accepting";
	LOG_I(m_strStatus);
	int socketNew;
	struct sockaddr_in clientAddr;
	int c = sizeof(struct sockaddr_in);

	while ((socketNew = accept(m_socket, (struct sockaddr *) &clientAddr,
			(socklen_t*) &c)) >= 0)
	{
		LOG_I("Accepted new connection");

		if (m_lSocket.size() >= m_nSocket)
		{
			LOG_I("Incoming socket number reached limit");

			//clear up disconnected sockets
			auto itr = m_lSocket.begin();
			while (itr != m_lSocket.end())
			{
				_TCPsocket* pSocket = *itr;
				if (!pSocket->m_bConnected)
				{
					itr = m_lSocket.erase(itr);
					pSocket->complete();
					delete pSocket;
					LOG_I("Deleted disconnected socket");
				}
				else
				{
					itr++;
				}
			}

			if (m_lSocket.size() >= m_nSocket)
				continue;
		}

		_TCPsocket* pSocket = new _TCPsocket();
		if (!pSocket)
		{
			LOG_E("_socket create failed");
			continue;
		}
		LOG_I("Created new _socket");

		pSocket->init(m_pKiss);
		struct sockaddr_in *pAddr = (struct sockaddr_in *) &clientAddr;
		pSocket->m_strAddr = inet_ntoa(pAddr->sin_addr);
		pSocket->m_socket = socketNew;
		pSocket->m_bConnected = true;
		pSocket->m_bClient = false;

		struct timeval timeout;
		timeout.tv_sec = pSocket->m_timeoutRecv / USEC_1SEC;
		timeout.tv_usec = pSocket->m_timeoutRecv % USEC_1SEC;
		setsockopt(socketNew, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		if (!pSocket->start())
		{
			LOG_E("Socket start failed");
			delete pSocket;
			continue;
		}

		m_lSocket.push_back(pSocket);
		LOG_I("Allocated new socket");
	}

	close(m_socket);

	if (socketNew < 0)
	{
		m_strStatus = "Accept failed";
		LOG_E(m_strStatus + ": "<<errno);
		return false;
	}

	return true;
}

_TCPsocket* _webSocketServer::getFirstSocket(void)
{
	IF_N(m_lSocket.empty());

	return m_lSocket.front();
}

void _webSocketServer::complete(void)
{
	close(m_socket);
	this->_ThreadBase::complete();

	for (auto itr = m_lSocket.begin(); itr != m_lSocket.end(); itr++)
	{
		delete (_TCPsocket*)*itr;
	}

	m_lSocket.clear();
}

bool _webSocketServer::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg = "Server port: " + i2str(m_listenPort) + " STATUS: " + m_strStatus;
	pWin->addMsg(&msg);

	pWin->tabNext();
	for (auto itr = m_lSocket.begin(); itr != m_lSocket.end(); ++itr)
	{
		((_TCPsocket*) *itr)->draw();
	}
	pWin->tabPrev();

	return true;
}

}
