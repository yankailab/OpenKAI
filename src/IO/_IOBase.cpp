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
}

_IOBase::~_IOBase()
{
	m_ioR.dest();
	m_ioW.dest();
}

bool _IOBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	int n;

	n = 0;
	F_INFO(pK->v("nBufIOR", &n));
	IF_F(!m_ioR.init(n));

	n = 0;
	F_INFO(pK->v("nBufIOW", &n));
	IF_F(!m_ioW.init(n));

	return true;
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

	m_ioW.write(pBuf,nB);
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
	if(m_ioStatus != io_opened)return -1;
	if(pBuf == NULL)return -1;
	if(nB <= 0)return 0;

	return m_ioR.read(pBuf, nB);
}

IO_TYPE _IOBase::ioType(void)
{
	return m_ioType;
}

void _IOBase::close(void)
{
	m_ioR.close();
	m_ioW.close();

	m_ioStatus = io_closed;
	LOG_I("Closed");
}


}
