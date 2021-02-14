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
	m_dir = "/home/";
	m_subDir = "";
	m_iImg = 0;
	m_iImgSet = 0;
	m_nImgPerSet = 6;
}

_HiphenServer::~_HiphenServer()
{
}

bool _HiphenServer::init(void* pKiss)
{
	IF_F(!_TCPserver::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("dir",&m_dir);
	pK->v("subDir",&m_subDir);
	pK->v("nImgPerSet",&m_nImgPerSet);

	if(m_subDir.empty())
		m_subDir = m_dir + tFormat() + "/";
	else
		m_subDir = m_dir + m_subDir;

	string cmd = "mkdir " + m_subDir;
	system(cmd.c_str());

	return true;
}

bool _HiphenServer::start(void)
{
    NULL_F(m_pT);
    return m_pT->start(getUpdate, this);
}

void _HiphenServer::update(void)
{
	while(m_pT->bRun())
	{
		if(!handler())
		{
			m_pT->sleepT (USEC_1SEC);
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
		cleanupClient();
		IF_CONT(m_lSocket.size() >= m_nSocket);

		_HiphenIMG* pImgSock = new _HiphenIMG();
		IF_CONT(!pImgSock);
		pImgSock->init(m_pKiss);
		struct sockaddr_in *pAddr = (struct sockaddr_in *) &clientAddr;
		pImgSock->m_strAddr = inet_ntoa(pAddr->sin_addr);
		pImgSock->m_socket = socketNew;
		pImgSock->m_ioStatus = io_opened;
		pImgSock->m_bClient = false;
		pImgSock->m_dir = m_subDir;

		if (!pImgSock->start())
		{
			delete pImgSock;
			continue;
		}

		m_lSocket.push_back(pImgSock);
		m_iImg++;
		m_iImgSet = m_iImg / m_nImgPerSet;
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

int _HiphenServer::getImgSetIdx(void)
{
	return m_iImgSet;
}

string _HiphenServer::getDir(void)
{
	return m_subDir;
}

}
