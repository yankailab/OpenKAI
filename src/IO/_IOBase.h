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

#define N_IOBUF_MIN 128
#define N_IOBUF_MAX 10240

namespace kai
{

enum IO_TYPE
{
	io_none,
	io_serialport,
	io_file,
	io_tcp,
	io_udp
};

enum IO_STATUS
{
	io_unknown,
	io_closed,
	io_opened
};

struct IO_STREAM
{
	int m_nBuf;
	uint8_t* m_pBuf;
	std::queue<uint8_t> m_queue;
	pthread_mutex_t m_mutex;

	bool init(int nBuf)
	{
		m_nBuf = constrain(nBuf,N_IOBUF_MIN,N_IOBUF_MAX);
		m_pBuf = new uint8_t[m_nBuf];
		NULL_F(m_pBuf);

		pthread_mutex_init(&m_mutex, NULL);
		return true;
	}

	void close(void)
	{
		while (!m_queue.empty())
			m_queue.pop();
	}

	void dest(void)
	{
		DEL(m_pBuf);
		pthread_mutex_destroy (&m_mutex);
	}

	void write(uint8_t* pBuf, int nB)
	{
		pthread_mutex_lock(&m_mutex);

		for (int i = 0; i < nB; i++)
			m_queue.push(pBuf[i]);

		pthread_mutex_unlock(&m_mutex);
	}

	int read(uint8_t* pBuf, int nB)
	{
		pthread_mutex_lock(&m_mutex);

		int i=0;
		while(!m_queue.empty() && i<nB)
		{
			pBuf[i++] = m_queue.front();
			m_queue.pop();
		}

		pthread_mutex_unlock(&m_mutex);

		return i;
	}

	int que2buf(void)
	{
		if(m_queue.empty())
			return 0;

		pthread_mutex_lock(&m_mutex);
		int nB = 0;
		while (!m_queue.empty() && nB < m_nBuf)
		{
			m_pBuf[nB++] = m_queue.front();
			m_queue.pop();
		}
		pthread_mutex_unlock(&m_mutex);

		return nB;
	}

	void buf2que(int nB)
	{
		pthread_mutex_lock(&m_mutex);
		for(int i=0; i<nB; i++)
		{
			m_queue.push(m_pBuf[i]);
		}
		pthread_mutex_unlock(&m_mutex);
	}
};

class _IOBase: public _ThreadBase
{
public:
	_IOBase();
	virtual ~_IOBase();

	bool init(void* pKiss);
	virtual bool open(void);
	virtual bool isOpen(void);

	int  read(uint8_t* pBuf, int nB);
	bool write(uint8_t* pBuf, int nB);
	bool writeLine(uint8_t* pBuf, int nB);

	virtual void close(void);
	virtual IO_TYPE ioType(void);

public:
	IO_TYPE		m_ioType;
	IO_STATUS	m_ioStatus;

	IO_STREAM	m_ioR;
	IO_STREAM	m_ioW;
};

}

#endif
