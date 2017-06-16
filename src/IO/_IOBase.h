/*
 * _IOBase.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef SRC_NETWORK_IOBase_H_
#define SRC_NETWORK_IOBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"

#define N_BUF_IO 128

namespace kai
{

class _IOBase: public _ThreadBase
{
public:
	_IOBase();
	virtual ~_IOBase();

	bool init(void* pKiss);

	bool write(uint8_t* pBuf, int nByte);
	int  read(uint8_t* pBuf, int nByte);

	int writeIO(void);
	void readIO(int nRead);
	void close(void);

public:
	int m_nBufIO;
	uint8_t* m_pBufIO;

	std::queue<uint8_t> m_queWriteIO;
	std::queue<uint8_t> m_queReadIO;

	pthread_mutex_t m_mutexWriteIO;
	pthread_mutex_t m_mutexReadIO;

};

}

#endif
