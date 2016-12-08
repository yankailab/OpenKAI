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

#define N_PEER 1

namespace kai
{

struct INST_PEER
{
	string m_instName;
	_peer** m_ppPeer;
	_peer* m_pPeer;

	void init(void)
	{
		m_instName = "";
		m_pPeer = NULL;
		m_ppPeer = NULL;
	}
};

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

	bool registerInstPeer(string* pName, _peer** ppPeer);
private:
	bool handler(void);
	INST_PEER* getInstPeer(void);
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

	int			m_nInstPeer;
	INST_PEER	m_pInstPeer[N_PEER];

};

} /* namespace kai */

#endif /* SRC_NETWORK_SERVERE_H_ */
