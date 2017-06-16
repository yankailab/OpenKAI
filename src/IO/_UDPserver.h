/*
 * _UDPserver.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef SRC_NETWORK_UDPserver_H_
#define SRC_NETWORK_UDPserver_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"
#include "../IO/IO.h"
#include "_TCPsocket.h"

#define N_LISTEN 1
#define N_SOCKET 1

namespace kai
{

class _UDPserver: public _ThreadBase
{
public:
	_UDPserver();
	virtual ~_UDPserver();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void complete(void);
	bool draw(void);
	_TCPsocket* getFirstSocket(void);

private:
	bool handler(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_UDPserver*) This)->update();
		return NULL;
	}

public:
	uint16_t	m_listenPort;
	int			m_nListen;
	string		m_strStatus;

	int m_socket;
	struct sockaddr_in m_serverAddr;

	list<_TCPsocket*> m_lSocket;
	int			m_nSocket;

};

}

#endif
