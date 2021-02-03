/*
 * _IOBase.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO_IOBase_H_
#define OpenKAI_src_IO_IOBase_H_

#include "../Base/_ModuleBase.h"

#define N_IO_BUF 512

namespace kai
{

enum IO_TYPE
{
	io_none,
	io_serialPort,
	io_file,
	io_tcp,
	io_udp,
	io_webSocket
};

enum IO_STATUS
{
	io_unknown,
	io_closed,
	io_opened
};

struct IO_FIFO
{
	uint8_t *m_pB;
	int m_nB;
	int m_nData;
	int m_iIn;
	int m_iOut;
	pthread_mutex_t m_mutex;

	bool init(int nB)
	{
		m_pB = new uint8_t[nB];
		NULL_F(m_pB);
		m_nB = nB;

		pthread_mutex_init(&m_mutex, NULL);

		clear();

		return true;
	}

	void release(void)
	{
		DEL(m_pB);
		pthread_mutex_destroy(&m_mutex);
	}

	void clear(void)
	{
		m_iIn = 0;
		m_iOut = 0;
		m_nData = 0;
	}

	bool input(uint8_t* pB, int nB)
	{
		IF_F(nB <= 0);
		NULL_F(pB);
		int bResult = false;

		pthread_mutex_lock(&m_mutex);

		if(m_nData + nB <= m_nB)
		{
			int iB = 0;
			if(m_iIn + nB > m_nB)
			{
				iB = m_nB - m_iIn;
				memcpy(&m_pB[m_iIn], pB, iB);
				m_iIn = 0;
			}

			int n = nB - iB;
			memcpy(&m_pB[m_iIn], &pB[iB], n);
			m_iIn += n;
			m_nData += nB;

			bResult = true;
		}

		pthread_mutex_unlock(&m_mutex);

		return bResult;
	}

	int output(uint8_t* pB, int nB)
	{
		if(nB <= 0)return 0;
		if(pB == NULL)return -1;

		pthread_mutex_lock(&m_mutex);

		int nO = nB;
		if(nO > m_nData)nO = m_nData;
		if(nO > 0)
		{
			int iB = 0;
			if(m_iOut + nO > m_nB)
			{
				iB = m_nB - m_iOut;
				memcpy(pB, &m_pB[m_iOut], iB);
				m_iOut = 0;
			}

			int n = nO - iB;
			memcpy(&pB[iB], &m_pB[m_iOut], n);
			m_iOut += n;
			m_nData -= nO;
		}

		pthread_mutex_unlock(&m_mutex);

		return nO;
	}

};

class _IOBase: public _ModuleBase
{
public:
	_IOBase();
	virtual ~_IOBase();

	virtual bool init(void* pKiss);
	virtual bool open(void);
	virtual bool isOpen(void);
	virtual void close(void);
	virtual void draw(void);
	virtual IO_TYPE ioType(void);

	virtual int  read(uint8_t* pBuf, int nB);
	virtual bool write(uint8_t* pBuf, int nB);
	virtual bool writeLine(uint8_t* pBuf, int nB);

public:
	IO_TYPE		m_ioType;
	IO_STATUS	m_ioStatus;

	int			m_nFIFO;
	IO_FIFO		m_fifoW;
	IO_FIFO		m_fifoR;

	pthread_t m_rThreadID;
	bool	m_bRThreadON;

};

}
#endif
