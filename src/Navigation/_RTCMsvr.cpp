/*
 * _RTCMsvr.cpp
 *
 *  Created on: Jun 3, 2020
 *      Author: yankai
 */

#include "_RTCMsvr.h"

namespace kai
{

	_RTCMsvr::_RTCMsvr()
	{
	}

	_RTCMsvr::~_RTCMsvr()
	{
	}

	int _RTCMsvr::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _RTCMsvr::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		// n = "";
		// pK->v("_IObase", &n);
		// m_pIO = (_IObase *)(pK->findModule(n));
		// NULL__(m_pIO, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _RTCMsvr::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _RTCMsvr::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		return this->_ModuleBase::check();
	}

	void _RTCMsvr::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			send();
		}
	}

	void _RTCMsvr::send(void)
	{
		IF_(check() != OK_OK);
	}

	void _RTCMsvr::updateR(void)
	{
		RTCM_BUF rCMD;

		while (m_pTr->bAlive())
		{
			IF_CONT(!readCMD(&rCMD));

			handleCMD(rCMD);
			rCMD.clear();
			m_nCMDrecv++;
		}
	}

	bool _RTCMsvr::readCMD(RTCM_BUF *pCmd)
	{
		IF_F(check() != OK_OK);
		NULL_F(pCmd);

		if (m_nRead == 0)
		{
			m_nRead = m_pIO->read(m_pBuf, PB_N_BUF);
			IF_F(m_nRead <= 0);
			m_iRead = 0;
		}

		while (m_iRead < m_nRead)
		{
			bool r = pCmd->input(m_pBuf[m_iRead++]);
			if (m_iRead == m_nRead)
			{
				m_iRead = 0;
				m_nRead = 0;
			}

			IF__(r, true);
		}

		return false;
	}

	void _RTCMsvr::handleCMD(const RTCM_BUF &cmd)
	{
	}

	void _RTCMsvr::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		if (!m_pIO->bOpen())
		{
			((_Console *)pConsole)->addMsg("Not connected");
			return;
		}
	}

}
