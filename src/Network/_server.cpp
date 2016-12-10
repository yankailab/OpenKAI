/*
 * _server.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_server.h"

namespace kai
{

_server::_server()
{
	_ThreadBase();

	m_socket = 0;
	m_listenPort = 8888;
	m_nListen = N_LISTEN;
	m_strStatus = "";
	m_lSocket.clear();

}

_server::~_server()
{
	complete();
}

bool _server::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("listenPort", (int*)&m_listenPort));
	F_INFO(pK->v("nlisten", &m_nListen));

	return true;
}

bool _server::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _server::start(void)
{
	CHECK_T(m_bThreadON);

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

void _server::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		handler();

		this->autoFPSto();
	}

}

bool _server::handler(void)
{
	//Create socket
	m_strStatus = "Creating socket";
	LOG(INFO)<<m_strStatus;
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_F(m_socket == -1);

	//Prepare the sockaddr_in structure
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.s_addr = INADDR_ANY;
	m_serverAddr.sin_port = htons(m_listenPort);

	//Bind
	m_strStatus = "Binding";
	LOG(INFO)<<m_strStatus;
	if(bind(m_socket, (struct sockaddr *) &m_serverAddr, sizeof(m_serverAddr)) < 0)
	{
		m_strStatus = "Binding failed";
		close(m_socket);
	}

	//Listen
	m_strStatus = "Listening";
	LOG(INFO)<<m_strStatus;
	listen(m_socket, m_nListen);

	//Accept incoming connection
	m_strStatus = "Accepting";
	LOG(INFO)<<m_strStatus;
	int socketNew;
	struct sockaddr_in clientAddr;
	int c = sizeof(struct sockaddr_in);

	while ((socketNew = accept(m_socket,
								(struct sockaddr *) &clientAddr,
								(socklen_t*) &c)))
	{
		LOG(INFO)<<"Accepted new connection";

		_socket* pSocket = new _socket();
		if(!pSocket)
		{
			LOG(ERROR)<<"_socket creat failed";
			continue;
		}

		struct sockaddr_in *pAddr = (struct sockaddr_in *)&clientAddr;
		pSocket->m_strAddr = inet_ntoa(pAddr->sin_addr);
		pSocket->m_socket = socketNew;
		pSocket->m_bConnected = true;
		pSocket->m_bClient = false;
		if(!pSocket->start())
		{
			delete pSocket;
			continue;
		}

		m_lSocket.push_back(pSocket);
		LOG(INFO)<<"Allocated new socket";
	}

	close(m_socket);

	if (socketNew < 0)
	{
		m_strStatus = "Accept failed";
		LOG(INFO)<<m_strStatus;
		return false;
	}

	return true;
}

_socket* _server::getFirstSocket(void)
{
	while(!m_lSocket.empty())
	{
		_socket* pSocket = m_lSocket.front();
		if(pSocket->m_bConnected)return pSocket;

		pSocket->complete();
		delete pSocket;
		m_lSocket.pop_front();
	}

	return NULL;
}

void _server::complete(void)
{
	close(m_socket);
	this->_ThreadBase::complete();
	pthread_cancel(m_threadID);

	for(auto itr = m_lSocket.begin(); itr != m_lSocket.end(); itr++)
	{
		((_socket*)*itr)->complete();
	}

	m_lSocket.clear();
}

bool _server::draw(Frame* pFrame, vInt4* pTextPos)
{
	CHECK_F(!this->_ThreadBase::draw(pFrame, pTextPos));

	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "Server port: " + i2str(m_listenPort)
					+ " STATUS: " + m_strStatus,
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;

	pTextPos->m_x += SHOW_TAB_PIX;
	for(auto itr = m_lSocket.begin(); itr != m_lSocket.end(); ++itr)
	{
		((_socket*)*itr)->draw(pFrame,pTextPos);
	}
	pTextPos->m_x -= SHOW_TAB_PIX;

	return true;
}

}
