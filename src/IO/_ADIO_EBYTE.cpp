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
	}

	_ADIO_EBYTE::~_ADIO_EBYTE()
	{
		close();
	}

	bool _ADIO_EBYTE::loadConfig(void)
	{
		IF_F(!this->_ADIObase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "iID", m_iID);

		return true;
	}

	bool _ADIO_EBYTE::saveConfig(bool bExport)
	{
		IF_F(!_ADIObase::saveConfig(false));

		json &j = *m_pJ;
		j["iID"] = m_iID;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _ADIO_EBYTE::link(void)
	{
		IF_F(!this->_ADIObase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_Modbus", n);
		m_pMB = (_Modbus *)(m_pM->findModule(n));
		NULL_F(m_pMB);

		return true;
	}

	bool _ADIO_EBYTE::open(void)
	{
		return true;
	}

	void _ADIO_EBYTE::close(void)
	{
	}

	bool _ADIO_EBYTE::check(void)
	{
		NULL_F(m_pMB);
		IF_F(!m_pMB->bOpen());

		return this->_ADIObase::check();
	}

	bool _ADIO_EBYTE::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _ADIO_EBYTE::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateW();
			updateR();
		}
	}

	void _ADIO_EBYTE::updateW(void)
	{
		IF_(!check());

		for (size_t i = 0; i < m_vPort.size(); i++)
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

		for (size_t i = 0; i < m_vPort.size(); i++)
		{
			ADIO_PORT *pP = &m_vPort[i];
			IF_CONT(pP->m_type > adio_inout);

			if (pP->bDigital())
			{
				uint8_t b = 0;
				m_pMB->readInputBits(m_iID, pP->m_addr, 1, &b);
				pP->m_vR = (b) ? 1 : 0;
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
	}

}
