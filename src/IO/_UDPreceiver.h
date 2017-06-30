/*
 * _UDPreceiver.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef SRC_IO_UDPreceiver_H_
#define SRC_IO_UDPreceiver_H_

#include "../Base/common.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"
#include "_IOBase.h"

#define TIMEOUT_RECV_USEC 1000

namespace kai
{

class _UDPreceiver: public _IOBase
{
public:
	_UDPreceiver();
	virtual ~_UDPreceiver();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void close(void);
	void complete(void);
	bool draw(void);

	bool open(void);

private:
	void readIO(void);
	void writeIO(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_UDPreceiver*) This)->update();
		return NULL;
	}

public:
	sockaddr_in m_sAddr;
	unsigned int m_nSAddr;
	string	m_strAddr;
	uint16_t m_port;
	int m_socket;
	uint32_t m_timeoutRecv;

};

}

#endif
