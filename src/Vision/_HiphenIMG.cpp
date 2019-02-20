/*
 * _HiphenIMG.cpp
 *
 *  Created on: Feb 19, 2019
 *      Author: yankai
 */

#include "_HiphenIMG.h"

namespace kai
{

_HiphenIMG::_HiphenIMG()
{
}

_HiphenIMG::~_HiphenIMG()
{
}

bool _HiphenIMG::init(void* pKiss)
{
	IF_F(!_TCPserver::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

//	KISSm(pK, nSave);

	return true;
}

bool _HiphenIMG::start(void)
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

void _HiphenIMG::update(void)
{
	while (m_bThreadON)
	{
		if(!handler())
		{
			this->sleepTime(USEC_1SEC);
		}
	}
}

bool _HiphenIMG::handler(void)
{
	//Create socket
	m_strStatus = "Creating socket";
	LOG_I(m_strStatus);
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	IF_F(m_socket == -1);

	//Prepare the sockaddr_in structure
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.s_addr = INADDR_ANY;
	m_serverAddr.sin_port = htons(m_listenPort);

	int yes = 1;
	setsockopt(m_socket,
	   SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

	//Bind
	m_strStatus = "Binding";
	LOG_I(m_strStatus);
	if (bind(m_socket, (struct sockaddr *) &m_serverAddr, sizeof(m_serverAddr)) < 0)
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

	while ((socketNew = accept(m_socket, (struct sockaddr *) &clientAddr, (socklen_t*) &c)) >= 0)
	{
		LOG_I("Accepted new connection");

		if (m_lSocket.size() >= m_nSocket)
		{
			//clear up disconnected sockets
			auto itr = m_lSocket.begin();
			while (itr != m_lSocket.end())
			{
				_TCPclient* pSocket = *itr;
				if(pSocket->m_ioStatus != io_opened)
				{
					itr = m_lSocket.erase(itr);
					delete pSocket;
					LOG_I("Deleted disconnected socket");
				}
				else
				{
					itr++;
				}
			}

			if(m_lSocket.size() >= m_nSocket)
			{
				LOG_I("Incoming socket number reached limit");
			}
		}

		_TCPclient* pSocket = new _TCPclient();
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
		pSocket->m_ioStatus = io_opened;
		pSocket->m_bClient = false;

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
		LOG_E(m_strStatus + ": " + i2str(errno));
		return false;
	}

	return true;
}

}
