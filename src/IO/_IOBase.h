/*
 * _IOBase.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO_IOBase_H_
#define OpenKAI_src_IO_IOBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"

#define N_IO_BUF 1024

namespace kai
{

enum IO_TYPE
{
	io_none,
	io_serialport,
	io_file,
	io_tcp,
	io_udp,
	io_fifo
};

enum IO_STATUS
{
	io_unknown,
	io_closed,
	io_opened
};

struct IO_BUF
{
	uint8_t m_pB[N_IO_BUF];
	int m_nB;

	void init(void)
	{
		m_nB = 0;
	}

	bool bEmpty(void)
	{
		return (m_nB<=0);
	}
};

class _IOBase: public _ThreadBase
{
public:
	_IOBase();
	virtual ~_IOBase();

	virtual bool init(void* pKiss);
	virtual bool open(void);
	virtual bool isOpen(void);
	virtual void close(void);
	virtual void reset(void);
	virtual IO_TYPE ioType(void);

	int  read(uint8_t* pBuf, int nB);
	bool write(uint8_t* pBuf, int nB);
	bool writeLine(uint8_t* pBuf, int nB);

	bool bEmptyW(void);
	void toBufW(IO_BUF* pB);
	void toQueR(IO_BUF* pB);

public:
	IO_TYPE		m_ioType;
	IO_STATUS	m_ioStatus;

	std::queue<IO_BUF> m_queW;
	std::queue<IO_BUF> m_queR;

	pthread_mutex_t m_mutexW;
	pthread_mutex_t m_mutexR;

};

}

#endif
