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
		m_ioType = io_none;
		m_ioStatus = io_unknown;
		m_nFIFO = 1280000;
	}

	_IObase::~_IObase()
	{
		m_fifoW.release();
	}

	int _IObase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nFIFO", &m_nFIFO);
		IF__(!m_fifoW.init(m_nFIFO), OK_ERR_ALLOCATION);

		return OK_OK;
	}

	int _IObase::link(void)
	{
		CHECK_(this->_ModuleBase::link());

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

	int _IObase::read(uint8_t *pBuf, int nB)
	{
		if (m_ioStatus != io_opened)
			return -1;

		return 0;
	}

	IO_STATUS _IObase::getIOstatus(void)
	{
		return m_ioStatus;
	}

	void _IObase::setIOstatus(IO_STATUS s)
	{
		m_ioStatus = s;
	}

	void _IObase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		((_Console *)pConsole)->addMsg("nFifoW=" + i2str(m_fifoW.m_nData), 0);
	}

}
