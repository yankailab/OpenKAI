/*
 * _TCPserver.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__TCPserver_H_
#define OpenKAI_src_IO__TCPserver_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"
#include "../IO/_IOBase.h"
#include "_TCPclient.h"

#define N_LISTEN 1
#define N_SOCKET 1

namespace kai
{

class _TCPserver: public _ThreadBase
{
public:
	_TCPserver();
	virtual ~_TCPserver();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	_TCPclient* getFirstSocket(void);

private:
	bool handler(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_TCPserver*) This)->update();
		return NULL;
	}

public:
	uint16_t	m_listenPort;
	int			m_nListen;
	string		m_strStatus;

	int m_socket;
	struct sockaddr_in m_serverAddr;

	list<_TCPclient*> m_lSocket;
	int			m_nSocket;

};

}

#endif
