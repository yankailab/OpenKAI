/*
 * _TCPserver.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_TCPserver.h"

namespace kai
{

	_TCPserver::_TCPserver()
	{
		m_socket = 0;
		m_listenPort = 8888;
		m_nListen = N_LISTEN;
		m_nSocket = N_SOCKET;
		m_lClient.clear();
	}

	_TCPserver::~_TCPserver()
	{
		close(m_socket);

		for (auto itr = m_lClient.begin(); itr != m_lClient.end(); itr++)
		{
			delete (_TCPclient *)*itr;
		}

		m_lClient.clear();
	}

	bool _TCPserver::init(const json& j)
	{
		IF_F(!this->_ModuleBase::init(j));

		= j.value<uint16_t>("port", &m_listenPort);
		= j.value<int>("nListen", &m_nListen);
		= j.value<unsigned int>("nSocket", &m_nSocket);

		return true;
	}

	bool _TCPserver::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _TCPserver::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!handler())
			{
				m_pT->sleepT(SEC_2_USEC);
			}
		}
	}

	bool _TCPserver::handler(void)
	{
		IF_F(!setup());

		int socketNew;
		struct sockaddr_in clientAddr;
		int c = sizeof(struct sockaddr_in);

		while ((socketNew = accept(m_socket, (struct sockaddr *)&clientAddr, (socklen_t *)&c)) >= 0)
		{
			cleanupClient();
			IF_CONT(m_lClient.size() >= m_nSocket);

			_TCPclient *pClient = new _TCPclient();
			IF_CONT(!pClient);
			pClient->init(m_pKiss);
			struct sockaddr_in *pAddr = (struct sockaddr_in *)&clientAddr;
			pClient->m_strAddr = inet_ntoa(pAddr->sin_addr);
			pClient->m_socket = socketNew;
			pClient->m_bClient = false;
			pClient->setIOstatus(io_opened);

			if (!pClient->start())
			{
				delete pClient;
				continue;
			}

			m_lClient.push_back(pClient);
		}

		close(m_socket);
		return true;
	}

	void _TCPserver::cleanupClient(void)
	{
		auto itr = m_lClient.begin();
		while (itr != m_lClient.end())
		{
			_TCPclient *pClient = *itr;
			if (pClient->getIOstatus() != io_opened)
			{
				itr = m_lClient.erase(itr);
				delete pClient;
				LOG_I("Deleted disconnected socket");
			}
			else
			{
				itr++;
			}
		}
	}

	bool _TCPserver::setup(void)
	{
		//Create socket
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		IF_F(m_socket < 0);

		//Prepare the sockaddr_in structure
		m_serverAddr.sin_family = AF_INET;
		m_serverAddr.sin_addr.s_addr = INADDR_ANY;
		m_serverAddr.sin_port = htons(m_listenPort);

		int yes = 1;
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

		//Bind
		if (bind(m_socket, (struct sockaddr *)&m_serverAddr, sizeof(m_serverAddr)) < 0)
		{
			close(m_socket);
			return false;
		}

		//Listen
		listen(m_socket, m_nListen);

		return true;
	}

	_TCPclient *_TCPserver::getFirstSocket(void)
	{
		IF__(m_lClient.empty(), nullptr);

		return m_lClient.front();
	}

	void _TCPserver::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		string msg = "Server port: " + i2str(m_listenPort);
		((_Console *)pConsole)->addMsg(msg);

		for (auto itr = m_lClient.begin(); itr != m_lClient.end(); ++itr)
		{
			((_TCPclient *)*itr)->console(pConsole);
		}
	}

}
