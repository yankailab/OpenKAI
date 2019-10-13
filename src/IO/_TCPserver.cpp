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
	m_lSocket.clear();
}

_TCPserver::~_TCPserver()
{
	close(m_socket);

	for (auto itr = m_lSocket.begin(); itr != m_lSocket.end(); itr++)
	{
		delete (_TCPclient*)*itr;
	}

	m_lSocket.clear();
}

bool _TCPserver::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v<uint16_t>("listenPort", &m_listenPort);
	pK->v<int>("nListen", &m_nListen);
	pK->v<unsigned int>("nSocket", &m_nSocket);

	return true;
}

bool _TCPserver::start(void)
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

void _TCPserver::update(void)
{
	while (m_bThreadON)
	{
		if(!handler())
		{
			this->sleepTime(USEC_1SEC);
		}
	}
}

bool _TCPserver::handler(void)
{
	IF_F(!setup());

	int socketNew;
	struct sockaddr_in clientAddr;
	int c = sizeof(struct sockaddr_in);

	while ((socketNew = accept(m_socket, (struct sockaddr *) &clientAddr, (socklen_t*) &c)) >= 0)
	{
		cleanupClient();
		IF_CONT(m_lSocket.size() >= m_nSocket);

		_TCPclient* pSocket = new _TCPclient();
		IF_CONT(!pSocket);
		pSocket->init(m_pKiss);
		struct sockaddr_in *pAddr = (struct sockaddr_in *) &clientAddr;
		pSocket->m_strAddr = inet_ntoa(pAddr->sin_addr);
		pSocket->m_socket = socketNew;
		pSocket->m_ioStatus = io_opened;
		pSocket->m_bClient = false;

		if (!pSocket->start())
		{
			delete pSocket;
			continue;
		}

		m_lSocket.push_back(pSocket);
	}

	close(m_socket);
	return true;
}

void _TCPserver::cleanupClient(void)
{
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
}

bool _TCPserver::setup(void)
{
	//Create socket
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	IF_F(m_socket == -1);

	//Prepare the sockaddr_in structure
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.s_addr = INADDR_ANY;
	m_serverAddr.sin_port = htons(m_listenPort);

	int yes = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

	//Bind
	if (bind(m_socket, (struct sockaddr *) &m_serverAddr, sizeof(m_serverAddr)) < 0)
	{
		close(m_socket);
		return false;
	}

	//Listen
	listen(m_socket, m_nListen);

	return true;
}

_TCPclient* _TCPserver::getFirstSocket(void)
{
	IF_N(m_lSocket.empty());

	return m_lSocket.front();
}

void _TCPserver::draw(void)
{
	this->_ThreadBase::draw();

	string msg = "Server port: " + i2str(m_listenPort);
	addMsg(msg);

	for (auto itr = m_lSocket.begin(); itr != m_lSocket.end(); ++itr)
	{
		((_TCPclient*) *itr)->draw();
	}
}

}
