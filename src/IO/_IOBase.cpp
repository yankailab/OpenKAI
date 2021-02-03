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
	m_rThreadID = 0;
	m_bRThreadON = false;
	m_ioType = io_none;
	m_ioStatus = io_unknown;
	m_nFIFO = 1280000;
}

_IOBase::~_IOBase()
{
	m_fifoW.release();
	m_fifoR.release();
}

bool _IOBase::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nFIFO",&m_nFIFO);

	IF_F(!m_fifoW.init(m_nFIFO));
	IF_F(!m_fifoR.init(m_nFIFO));

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

IO_TYPE _IOBase::ioType(void)
{
	return m_ioType;
}

void _IOBase::close(void)
{
	m_fifoW.clear();
	m_fifoR.clear();

	m_ioStatus = io_closed;
}

bool _IOBase::write(uint8_t* pBuf, int nB)
{
	IF_F(m_ioStatus != io_opened);

	IF_F(!m_fifoW.input(pBuf,nB));

	this->wakeUp();
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

	return m_fifoR.output(pBuf,nB);
}

void _IOBase::draw(void)
{
	this->_ModuleBase::draw();
	addMsg("nFifoW=" + i2str(m_fifoW.m_nData) + ", nFifoR=" + i2str(m_fifoR.m_nData), 1);
}

}
