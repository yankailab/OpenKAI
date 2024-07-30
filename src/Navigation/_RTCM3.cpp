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
		m_pIO = nullptr;
		m_nRead = 0;
		m_iRead = 0;
		m_msg = "";
	}

	_RTCM3::~_RTCM3()
	{
	}

	int _RTCM3::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n = "";
		pK->v("_IObase", &n);
		m_pIO = (_IObase *)(pK->findModule(n));
		NULL__(m_pIO, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _RTCM3::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _RTCM3::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			decode();
			m_msg = "";

			m_pT->autoFPSto();
		}
	}

	void _RTCM3::decode(void)
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
