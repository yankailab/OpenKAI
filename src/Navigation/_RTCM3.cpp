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
		m_pIO = NULL;
		m_nRead = 0;
		m_iRead = 0;
		m_msg = "";
	}

	_RTCM3::~_RTCM3()
	{
	}

	bool _RTCM3::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n = "";
		F_ERROR_F(pK->v("_IObase", &n));
		m_pIO = (_IObase *)(pK->getInst(n));
		IF_Fl(!m_pIO, "_IObase not found");

		return true;
	}

	bool _RTCM3::start(void)
	{
		NULL_F(m_pT);
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
