/*
 * _UDP.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__UDP_H_
#define OpenKAI_src_IO__UDP_H_

#include "../Base/common.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"
#include "_IOBase.h"

#define DEFAULT_UDP_PORT 19840

namespace kai
{

class _UDP: public _IOBase
{
public:
	_UDP();
	virtual ~_UDP();

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

	void updateW(void);
	static void* getUpdateThreadW(void* This)
	{
		((_UDP*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_UDP*) This)->updateR();
		return NULL;
	}

public:
	sockaddr_in m_sAddrR;
	unsigned int m_nSAddrR;
	sockaddr_in m_sAddrW;
	unsigned int m_nSAddrW;

	string	m_addrR;
	uint16_t m_portR;
	string	m_addrW;
	uint16_t m_portW;

	int m_socket;
};

}

#endif
