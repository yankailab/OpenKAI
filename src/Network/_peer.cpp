/*
 * _peer.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_peer.h"

namespace kai
{

_peer::_peer()
{
	_ThreadBase();

	pthread_mutex_init(&m_mutexSend, NULL);
	pthread_mutex_init(&m_mutexRecv, NULL);

	m_strStatus = "";
	m_strAddr = "";
	m_port = 0;
	m_bClient = true;
	m_bConnected = 0;
	m_socket = 0;
	m_iBuf = 0;
}

_peer::~_peer()
{

}

bool _peer::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("addr", &m_strAddr));
	F_INFO(pK->v("port", (int*)&m_port));

	m_strStatus = "Initialized";
	m_bConnected = false;
	m_iBuf = 0;
	m_bClient = true;

	return true;
}

bool _peer::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*) m_pKiss;

//	string iName = "";
//	F_INFO(pK->v("_Universe", &iName));
//	m_pUniverse = (_Universe*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _peer::start(void)
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

void _peer::update(void)
{
	if(m_bClient)
	{
		//client mode always trying to connect to the server
		while (m_bThreadON)
		{
			this->autoFPSfrom();

			if(connectServer())
			{
				msgHandler();
			}

			this->autoFPSto();
		}

//		close(m_socket);
	}
	else
	{
		//server side only maintain the connection until disconnected
		msgHandler();
	}

}

bool _peer::connectServer(void)
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_F(m_socket < 0);

	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(m_strAddr.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(m_port);
	m_strStatus = "Connecting....";

	if(connect(m_socket, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		close(m_socket);
		m_strStatus = "Connection failed";
		return false;
	}

	m_bConnected = true;
	m_strStatus = "CONNECTED";

	return true;
}

bool _peer::msgHandler(void)
{
	CHECK_F(m_bConnected==false);
	m_strStatus = "CONNECTED";

	int nRecv;
	while ((nRecv = recv(m_socket, &m_pBuf[m_iBuf], BUFSIZE - m_iBuf, 0)) > 0)
	{
		//TODO: when recv a msg, put its IP into Message for use in reply
		//TODO: if msgHandler of instance return 1, return the msg to peer
		//sendMsg();
	}

	CHECK_F(nRecv <= 0);

	return true;
}

bool _peer::sendMsg(Message* pMsg)
{
	NULL_F(pMsg);

	pthread_mutex_lock(&m_mutexSend);

	//Send some messages to the client
//	message = "Greetings! I am your connection handler\n";
//	write(m_socket, message, strlen(message));

	pthread_mutex_unlock(&m_mutexSend);

	return true;

}

void _peer::complete(void)
{
	close(m_socket);
	this->_ThreadBase::complete();
	pthread_cancel(m_threadID);
}


bool _peer::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);
	CHECK_F(pFrame->empty());
	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "Peer IP: " + m_strAddr + ":"+i2str(m_port)
					+ "; STATUS: " + m_strStatus
					+ ((m_bClient)?"; Client":"; Server"),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;

	return true;
}

} /* namespace kai */
