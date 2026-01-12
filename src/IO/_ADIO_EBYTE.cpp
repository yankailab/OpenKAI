/*
 * _ADIO_EBYTE.cpp
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#include "_ADIO_EBYTE.h"

namespace kai
{
	_ADIO_EBYTE::_ADIO_EBYTE()
	{
		m_pMB = nullptr;
		m_iID = 32;
	}

	_ADIO_EBYTE::~_ADIO_EBYTE()
	{
		close();
	}

	int _ADIO_EBYTE::init(const json& j)
	{
		CHECK_(this->_ADIObase::init(j));

		= j.value("iID", &m_iID);

		return true;
	}

	int _ADIO_EBYTE::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ADIObase::link(j, pM));

		string n;
		n = "";
		= j.value("_Modbus", &n);
		m_pMB = (_Modbus *)(pM->findModule(n));

		return true;
	}

	bool _ADIO_EBYTE::open(void)
	{
		return true;
	}

	void _ADIO_EBYTE::close(void)
	{
	}

	int _ADIO_EBYTE::check(void)
	{
		NULL__(m_pMB);
		IF__(!m_pMB->bOpen());

		return this->_ADIObase::check();
	}

	int _ADIO_EBYTE::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _ADIO_EBYTE::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!bOpen())
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			updateW();
			updateR();

		}
	}

	void _ADIO_EBYTE::updateW(void)
	{
		IF_(!check());

		for (int i = 0; i < m_vPort.size(); i++)
		{
			ADIO_PORT *pP = &m_vPort[i];
			IF_CONT(pP->m_type < adio_inout);

			if (pP->bDigital())
			{
				m_pMB->writeBit(m_iID, pP->m_addr, pP->writeD());
			}
			else
			{
//				m_pMB->writeBit(m_iID, pP->m_addr, pP->readD());
			}
		}
	}

	void _ADIO_EBYTE::updateR(void)
	{
		IF_(!check());

		for (int i = 0; i < m_vPort.size(); i++)
		{
			ADIO_PORT *pP = &m_vPort[i];
			IF_CONT(pP->m_type > adio_inout);

			if (pP->bDigital())
			{
				uint8_t b = 0;
				m_pMB->readInputBits(m_iID, pP->m_addr, 1, &b);
				pP->m_vR = (b)?1:0;
			}
			else
			{
//				m_pMB->writeBit(m_iID, pP->m_addr, pP->readD());
			}
		}

	}

	void _ADIO_EBYTE::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ADIObase::console(pConsole);

		//		((_Console *)pConsole)->addMsg();
	}

}
