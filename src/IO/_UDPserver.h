/*
 * _UDPserver.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__UDPserver_H_
#define OpenKAI_src_IO__UDPserver_H_

#include "../Base/common.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"
#include "_IOBase.h"

#define TIMEOUT_RECV_USEC 1000
#define DEFAULT_PORT_IN 19840

namespace kai
{

class _UDPserver: public _IOBase
{
public:
	_UDPserver();
	virtual ~_UDPserver();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void close(void);
	void reset(void);
	bool draw(void);

	bool open(void);

private:
	void readIO(void);
	void writeIO(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_UDPserver*) This)->update();
		return NULL;
	}

public:
	sockaddr_in m_sAddr;
	unsigned int m_nSAddr;
	uint16_t m_port;
	int m_socket;
	uint32_t m_timeoutRecv;

	sockaddr_in m_sAddrPeer;
	unsigned int m_nSAddrPeer;

	bool m_bReceiveOnly;



};

}

#endif
