/*
 * _HiphenServer.cpp
 *
 *  Created on: Feb 19, 2019
 *      Author: yankai
 */

#include "_HiphenServer.h"

namespace kai
{

_HiphenServer::_HiphenServer()
{
}

_HiphenServer::~_HiphenServer()
{
}

bool _HiphenServer::init(void* pKiss)
{
	IF_F(!_TCPserver::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _HiphenServer::start(void)
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

void _HiphenServer::update(void)
{
	while (m_bThreadON)
	{
		if(!handler())
		{
			this->sleepTime(USEC_1SEC);
		}
	}
}

bool _HiphenServer::handler(void)
{
	IF_F(!setup());

	int socketNew;
	struct sockaddr_in clientAddr;
	int c = sizeof(struct sockaddr_in);

	while ((socketNew = accept(m_socket, (struct sockaddr *) &clientAddr, (socklen_t*) &c)) >= 0)
	{
		if (m_lSocket.size() >= m_nSocket)
		{
			cleanupClient();
			IF_CONT(m_lSocket.size() >= m_nSocket);
		}

		_HiphenIMG* pSocket = new _HiphenIMG();
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

void _HiphenServer::cleanupClient(void)
{
	auto itr = m_lSocket.begin();
	while (itr != m_lSocket.end())
	{
		_HiphenIMG* pSocket = (_HiphenIMG*)*itr;
		if(pSocket->bComplete())
		{
			itr = m_lSocket.erase(itr);
			delete pSocket;
		}
		else
		{
			itr++;
		}
	}
}

}
