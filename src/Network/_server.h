/*
 * _server.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef SRC_NETWORK_SERVER_H_
#define SRC_NETWORK_SERVER_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"
#include "../IO/IO.h"
#include "_peer.h"

#define N_PEER 2

namespace kai
{

class _server: public _ThreadBase
{
public:
	_server();
	virtual ~_server();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void complete(void);
	bool draw(Frame* pFrame, vInt4* pTextPos);

	bool socketHandler(void);

private:
	int getFreePeer(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_server*) This)->update();
		return NULL;
	}

public:
	uint16_t	m_listenPort;
	int			m_nListen;
	string		m_strStatus;

	int m_socket;
	struct sockaddr_in m_serverAddr;

	_peer* m_ppPeer[N_PEER];

};

} /* namespace kai */

#endif /* SRC_NETWORK_SERVERE_H_ */
