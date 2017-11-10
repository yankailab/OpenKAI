/*
 * UDP.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef SRC_IO_UDP_H_
#define SRC_IO_UDP_H_

#include "../Base/common.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"
#include "_IOBase.h"
#include "_UDPclient.h"
#include "_UDPserver.h"

namespace kai
{

class _UDP: public _IOBase
{
public:
	_UDP();
	virtual ~_UDP();

	bool init(void* pKiss);
	bool link(void);
	void close(void);
	bool draw(void);

	int  read(uint8_t* pBuf, int nB);
	bool write(uint8_t* pBuf, int nB);
	bool writeLine(uint8_t* pBuf, int nB);

public:
	_UDPclient* m_pSender;
	_UDPserver* m_pReceiver;


};

}

#endif
