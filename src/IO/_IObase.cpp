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
		m_pTr = nullptr;

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

	int _IObase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nFIFO", &m_nFIFO);
		IF__(!m_fifoW.init(m_nFIFO), OK_ERR_ALLOCATION);
		IF__(!m_fifoR.init(m_nFIFO), OK_ERR_ALLOCATION);

		return OK_OK;
	}

	int _IObase::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		if (m_pTr)
		{
			CHECK_(m_pTr->link());
		}

		return OK_OK;
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

		NULL__(m_pT, true);
		m_pT->run();
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
