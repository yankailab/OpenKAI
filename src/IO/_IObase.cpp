/*
 * _IObase.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "_IObase.h"

namespace kai
{

	_IObase::_IObase()
	{
		m_pTr = NULL;

		m_ioType = io_none;
		m_ioStatus = io_unknown;
		m_nFIFO = 1280000;
	}

	_IObase::~_IObase()
	{
		DEL(m_pTr);

		m_fifoW.release();
		m_fifoR.release();
	}

	bool _IObase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nFIFO", &m_nFIFO);
		IF_F(!m_fifoW.init(m_nFIFO));
		IF_F(!m_fifoR.init(m_nFIFO));

		return true;
	}

	bool _IObase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		if (m_pTr)
		{
			IF_F(!m_pTr->link());
		}

		return true;
	}

	bool _IObase::open(void)
	{
		return false;
	}

	bool _IObase::bOpen(void)
	{
		return (m_ioStatus == io_opened);
	}

	IO_TYPE _IObase::ioType(void)
	{
		return m_ioType;
	}

	void _IObase::close(void)
	{
		m_fifoW.clear();
		m_fifoR.clear();

		m_ioStatus = io_closed;
	}

	bool _IObase::write(uint8_t *pBuf, int nB)
	{
		IF_F(m_ioStatus != io_opened);

		IF_F(!m_fifoW.input(pBuf, nB));

		NULL_T(m_pT);
		m_pT->wakeUp();
		return true;
	}

	bool _IObase::writeLine(uint8_t *pBuf, int nB)
	{
		const char pCRLF[] = "\x0d\x0a";

		IF_F(!write(pBuf, nB));
		return write((unsigned char *)pCRLF, 2);
	}

	int _IObase::read(uint8_t *pBuf, int nB)
	{
		if (m_ioStatus != io_opened)
			return -1;

		return m_fifoR.output(pBuf, nB);
	}

	void _IObase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		((_Console *)pConsole)->addMsg("nFifoW=" + i2str(m_fifoW.m_nData) + ", nFifoR=" + i2str(m_fifoR.m_nData), 0);
	}

}
