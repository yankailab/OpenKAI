/*
 * _RTCM3.cpp
 *
 *  Created on: Jun 3, 2020
 *      Author: yankai
 */

#include "_RTCM3.h"

namespace kai
{

	_RTCM3::_RTCM3()
	{
	}

	_RTCM3::~_RTCM3()
	{
	}

	int _RTCM3::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _RTCM3::link(void)
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

	int _RTCM3::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _RTCM3::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		return this->_ModuleBase::check();
	}

	void _RTCM3::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			send();
		}
	}

	void _RTCM3::send(void)
	{
		IF_(check() != OK_OK);
	}

	void _RTCM3::updateR(void)
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

	bool _RTCM3::readCMD(RTCM_BUF *pCmd)
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

	void _RTCM3::handleCMD(const RTCM_BUF &cmd)
	{
	}

	void _RTCM3::console(void *pConsole)
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
