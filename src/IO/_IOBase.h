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

enum IO_TYPE
{
	none,
	serialport,
	file,
	tcp,
	udp
};

enum IO_STATUS
{
	unknown,
	closed,
	opening
};

class _IOBase: public _ThreadBase
{
public:
	_IOBase();
	virtual ~_IOBase();

	bool init(void* pKiss);
	virtual bool open(void);
	virtual bool isOpen(void);

	bool write(uint8_t* pBuf, int nByte);
	int  read(uint8_t* pBuf, int nByte);

	int writeIO(void);
	void readIO(int nRead);

	virtual void close(void);
	virtual IO_TYPE type(void);

public:
	IO_TYPE		m_type;
	IO_STATUS	m_status;

	int m_nBufIO;
	uint8_t* m_pBufIO;

	std::queue<uint8_t> m_queWriteIO;
	std::queue<uint8_t> m_queReadIO;

	pthread_mutex_t m_mutexWriteIO;
	pthread_mutex_t m_mutexReadIO;

};

}

#endif
