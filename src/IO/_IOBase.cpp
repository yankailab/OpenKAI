/*
 * _IOBase.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "_IOBase.h"

namespace kai
{

_IOBase::_IOBase()
{
	pthread_mutex_init(&m_mutexWriteIO, NULL);
	pthread_mutex_init(&m_mutexReadIO, NULL);

	m_pBufIO = NULL;
	m_nBufIO = N_BUF_IO;

	m_type = none;
	m_status = unknown;
}

_IOBase::~_IOBase()
{
	pthread_mutex_destroy (&m_mutexWriteIO);
	pthread_mutex_destroy (&m_mutexReadIO);
}

bool _IOBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, nBufIO);
	if (m_nBufIO < N_BUF_IO)
		m_nBufIO = N_BUF_IO;
	m_pBufIO = new uint8_t[m_nBufIO];
	NULL_F(m_pBufIO);

	return true;
}

bool _IOBase::open(void)
{
	return false;
}

bool _IOBase::isOpen(void)
{
	return (m_status == opening);
}

int _IOBase::writeIO(void)
{
	if(m_queWriteIO.empty())
		return 0;

	pthread_mutex_lock(&m_mutexWriteIO);
	int nByte = 0;
	while (!m_queWriteIO.empty() && nByte < m_nBufIO)
	{
		m_pBufIO[nByte++] = m_queWriteIO.front();
		m_queWriteIO.pop();
	}
	pthread_mutex_unlock(&m_mutexWriteIO);

	return nByte;
}

void _IOBase::readIO(int nRead)
{
	pthread_mutex_lock(&m_mutexReadIO);
	for(int i=0; i<nRead; i++)
	{
		m_queReadIO.push(m_pBufIO[i]);
	}
	pthread_mutex_unlock(&m_mutexReadIO);
}

bool _IOBase::write(uint8_t* pBuf, int nByte)
{
	IF_F(nByte <= 0);
	NULL_F(pBuf);

	pthread_mutex_lock(&m_mutexWriteIO);

	for (int i = 0; i < nByte; i++)
		m_queWriteIO.push(pBuf[i]);

	pthread_mutex_unlock(&m_mutexWriteIO);

	return true;
}

int _IOBase::read(uint8_t* pBuf, int nByte)
{
	if(pBuf==NULL)return -1;
	if(nByte<=0)return 0;

	pthread_mutex_lock(&m_mutexReadIO);

	int i=0;
	while(!m_queReadIO.empty() && i<nByte)
	{
		pBuf[i++] = m_queReadIO.front();
		m_queReadIO.pop();
	}

	pthread_mutex_unlock(&m_mutexReadIO);

	return i;
}

IO_TYPE _IOBase::type(void)
{
	return m_type;
}

void _IOBase::close(void)
{
	while (!m_queWriteIO.empty())
		m_queWriteIO.pop();
	while (!m_queReadIO.empty())
		m_queReadIO.pop();
}


}
