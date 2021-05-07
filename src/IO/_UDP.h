/*
 * _UDP.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__UDP_H_
#define OpenKAI_src_IO__UDP_H_

#include "../Script/Kiss.h"
#include "../UI/_WindowCV.h"
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
	bool start(void);
	void close(void);
	void console(void* pConsole);
	bool open(void);

private:
	void updateW(void);
	static void* getUpdateW(void* This)
	{
		((_UDP*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateR(void* This)
	{
		((_UDP*) This)->updateR();
		return NULL;
	}

public:
    _Thread* m_pTr;
    
	string	m_addr;
	uint16_t m_port;

	sockaddr_in m_sAddr;
	unsigned int m_nSAddr;
	int m_socket;
};

}
#endif
