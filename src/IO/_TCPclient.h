/*
 * _TCPclient.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__TCPclient_H_
#define OpenKAI_src_IO__TCPclient_H_

#include "../Base/common.h"
#include "_IOBase.h"
#include "../Protocol/Peer.h"

namespace kai
{

class _TCPclient: public _IOBase
{
public:
	_TCPclient();
	virtual ~_TCPclient();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);
	bool draw(void);

private:
	void updateW(void);
	static void* getUpdateThreadW(void* This)
	{
		((_TCPclient*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_TCPclient*) This)->updateR();
		return NULL;
	}

public:
	struct sockaddr_in m_serverAddr;
	string	m_strAddr;
	uint16_t m_port;

	bool m_bClient;
	int m_socket;
};

}
#endif
