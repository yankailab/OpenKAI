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
	m_ioType = io_none;
	m_ioStatus = io_unknown;

	pthread_mutex_init(&m_mutexW, NULL);
	pthread_mutex_init(&m_mutexR, NULL);
}

_IOBase::~_IOBase()
{
	reset();
	pthread_mutex_destroy(&m_mutexW);
	pthread_mutex_destroy(&m_mutexR);
}

bool _IOBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

void _IOBase::reset(void)
{
	this->_ThreadBase::reset();
}

bool _IOBase::open(void)
{
	return false;
}

bool _IOBase::isOpen(void)
{
	return (m_ioStatus == io_opened);
}

bool _IOBase::write(uint8_t* pBuf, int nB)
{
	IF_F(m_ioStatus != io_opened);
	IF_F(nB <= 0);
	NULL_F(pBuf);

	IO_BUF ioB;
	int nW = 0;

	pthread_mutex_lock(&m_mutexW);

	while (nW < nB)
	{
		ioB.m_nB = nB - nW;
		if(ioB.m_nB > N_IO_BUF)
			ioB.m_nB = N_IO_BUF;

		memcpy(ioB.m_pB, &pBuf[nW], ioB.m_nB);
		nW += ioB.m_nB;

		m_queW.push(ioB);
	}

	pthread_mutex_unlock(&m_mutexW);

	return true;
}

bool _IOBase::writeLine(uint8_t* pBuf, int nB)
{
	const char pCRLF[] = "\x0d\x0a";

	IF_F(!write(pBuf, nB));
	return write((unsigned char*)pCRLF, 2);
}

int _IOBase::read(uint8_t* pBuf, int nB)
{
	if(m_ioStatus != io_opened)return 0;
	if(pBuf == NULL)return 0;
	if(nB < N_IO_BUF)return 0;
	if(m_queR.empty())return 0;

	pthread_mutex_lock(&m_mutexR);
	IO_BUF ioB = m_queR.front();
	m_queR.pop();
	pthread_mutex_unlock(&m_mutexR);

	memcpy(pBuf, ioB.m_pB, ioB.m_nB);

	return ioB.m_nB;
}

bool _IOBase::bEmptyW(void)
{
	return m_queW.empty();
}

void _IOBase::toBufW(IO_BUF* pB)
{
	NULL_(pB);

	if(m_queW.empty())
	{
		pB->init();
		return;
	}

	pthread_mutex_lock(&m_mutexW);

	*pB = m_queW.front();
	m_queW.pop();

	pthread_mutex_unlock(&m_mutexW);
}

void _IOBase::toQueR(IO_BUF* pB)
{
	NULL_(pB);
	IF_(pB->bEmpty());

	pthread_mutex_lock(&m_mutexR);

	m_queR.push(*pB);

	pthread_mutex_unlock(&m_mutexR);
}

IO_TYPE _IOBase::ioType(void)
{
	return m_ioType;
}

void _IOBase::close(void)
{
	while (!m_queW.empty())
		m_queW.pop();

	while (!m_queR.empty())
		m_queR.pop();

	m_ioStatus = io_closed;
}

}
