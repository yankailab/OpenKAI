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
	m_nListen = N_PEER;
	m_strStatus = "";
	m_nInstPeer = 0;

	for(int i=0;i<N_PEER;i++)
	{
		m_pInstPeer[i].init();
	}
}

_server::~_server()
{

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
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_F(m_socket == -1);

	//Prepare the sockaddr_in structure
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.s_addr = INADDR_ANY;
	m_serverAddr.sin_port = htons(m_listenPort);

	//Bind
	m_strStatus = "Binding";
	if(bind(m_socket, (struct sockaddr *) &m_serverAddr, sizeof(m_serverAddr)) < 0)
	{
		m_strStatus = "Binding failed";
		close(m_socket);
	}

	//Listen
	m_strStatus = "Listening";
	listen(m_socket, m_nListen);

	//Accept incoming connection
	m_strStatus = "Accepting";
	int socketNew;
	struct sockaddr_in clientAddr;
	int c = sizeof(struct sockaddr_in);

	while ((socketNew = accept(m_socket,
								(struct sockaddr *) &clientAddr,
								(socklen_t*) &c)))
	{
		INST_PEER* pInstPeer = getInstPeer();
		if(!pInstPeer)
		{
			LOG(ERROR)<<"Peer instance not found";
			continue;
		}

		struct sockaddr_in *pAddr = (struct sockaddr_in *)&clientAddr;
		pInstPeer->m_pPeer->m_strAddr = inet_ntoa(pAddr->sin_addr);
		pInstPeer->m_pPeer->m_socket = socketNew;
		pInstPeer->m_pPeer->m_bConnected = true;
		pInstPeer->m_pPeer->start();
		*pInstPeer->m_ppPeer = pInstPeer->m_pPeer;
	}

	close(m_socket);

	if (socketNew < 0)
	{
		m_strStatus = "Accept failed";
		return false;
	}

	return true;
}

INST_PEER* _server::getInstPeer(void)
{
	int i;
	INST_PEER* pInstPeer = NULL;
	for(i=0;i<m_nInstPeer;i++)
	{
		pInstPeer = &m_pInstPeer[i];	//temporal
	}

	_peer* pPeer = pInstPeer->m_pPeer;
	if(pPeer)
	{
		if(pPeer->m_bConnected)
		{
			LOG(ERROR)<<"Peer instance already in connection";
			return NULL;
		}

		*pInstPeer->m_ppPeer = NULL;
		pPeer->complete();
		delete pPeer;
	}

	pInstPeer->m_pPeer = new _peer();
	pPeer = pInstPeer->m_pPeer;
	pPeer->init(m_pKiss);
	pPeer->m_bClient = false;

	return pInstPeer;
}

bool _server::registerInstPeer(string* pName, _peer** ppPeer)
{
	NULL_F(pName);
	NULL_F(ppPeer);
	CHECK_F(m_nInstPeer >= N_PEER);

	m_pInstPeer[m_nInstPeer].m_instName = *pName;
	m_pInstPeer[m_nInstPeer].m_ppPeer = ppPeer;
	m_nInstPeer++;

	return true;
}

void _server::complete(void)
{
	close(m_socket);
	this->_ThreadBase::complete();
	pthread_cancel(m_threadID);

	for(int i=0;i<m_nInstPeer;i++)
	{
		m_pInstPeer[i].m_pPeer->complete();
	}
}

bool _server::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);
	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "Server port: " + i2str(m_listenPort)
					+ " STATUS: " + m_strStatus,
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;

	for(int i=0;i<m_nInstPeer;i++)
	{
		_peer* pPeer = m_pInstPeer[i].m_pPeer;
		if(!pPeer)continue;
		if(!pPeer->m_bConnected)continue;

		pPeer->draw(pFrame,pTextPos);
	}

	return true;
}

} /* namespace kai */
