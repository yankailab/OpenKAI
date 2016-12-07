/*
 * _peer.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef SRC_NETWORK_PEER_H_
#define SRC_NETWORK_PEER_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"
#include "../Message/Message.h"

#define BUFSIZE 512

namespace kai
{

class _peer: public _ThreadBase
{
public:
	_peer();
	virtual ~_peer();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void complete(void);
	bool draw(Frame* pFrame, vInt4* pTextPos);

	bool connectServer(void);
	bool sendMsg(Message* pMsg);

private:
	bool msgHandler(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_peer*) This)->update();
		return NULL;
	}

public:
	string	m_strStatus;

	string	m_strAddr;
	uint16_t m_port;

	bool m_bClient;
	bool m_bConnected;
	int m_socket;

	int	m_iBuf;
	uint8_t m_pBuf[BUFSIZE];

	struct sockaddr_in m_serverAddr;

	pthread_mutex_t m_mutexSend;
	pthread_mutex_t m_mutexRecv;


};

}

#endif
