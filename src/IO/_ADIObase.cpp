/*
 * _ADIObase.cpp
 *
 *  Created on: Jan 6, 2024
 *      Author: yankai
 */

#include "_ADIObase.h"

namespace kai
{

	_ADIObase::_ADIObase()
	{
	}

	_ADIObase::~_ADIObase()
	{
	}

	bool _ADIObase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		m_vPort.clear();
		const json *pJP = jK(j, "ports");
		IF__(!pJP || !pJP->is_array(), true);
		const json &jP = *pJP;

		for (auto it = jP.begin(); it != jP.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			ADIO_PORT port;
			port.clear();
			jKv(Ji, "bDigital", port.m_bDigital);
			jKv(Ji, "type", port.m_type);
			jKv(Ji, "addr", port.m_addr);
			jKv(Ji, "vW", port.m_vW);
			jKv(Ji, "vR", port.m_vR);

			m_vPort.push_back(port);
		}

		return true;
	}

	bool _ADIObase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		json &ports = j["ports"];
		if (!ports.is_array())
		{
			ports = json::array();
		}
		size_t iConfig = 0;
		for (const ADIO_PORT &port : m_vPort)
		{
			while (iConfig < ports.size() && !ports[iConfig].is_object())
			{
				++iConfig;
			}
			json &entry = ports[iConfig++];
			entry["bDigital"] = port.m_bDigital;
			entry["type"] = port.m_type;
			entry["addr"] = port.m_addr;
			entry["vW"] = port.m_vW;
			entry["vR"] = port.m_vR;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _ADIObase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		return true;
	}

	void _ADIObase::updateW(void)
	{
	}

	void _ADIObase::updateR(void)
	{
	}

	bool _ADIObase::writeD(int iPort, bool b)
	{
		IF_F(iPort < 0 || static_cast<size_t>(iPort) >= m_vPort.size());

		return m_vPort[iPort].writeD(b);
	}

	bool _ADIObase::writeA(int iPort, float v)
	{
		IF_F(iPort < 0 || static_cast<size_t>(iPort) >= m_vPort.size());

		return m_vPort[iPort].writeA(v);
	}

	bool _ADIObase::readD(int iPort)
	{
		IF_F(iPort < 0 || static_cast<size_t>(iPort) >= m_vPort.size());

		return m_vPort[iPort].readD();
	}

	float _ADIObase::readA(int iPort)
	{
		IF_F(iPort < 0 || static_cast<size_t>(iPort) >= m_vPort.size());

		return m_vPort[iPort].readA();
	}

	void _ADIObase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = ((_Console *)pConsole);
		string str;

		for (size_t i = 0; i < m_vPort.size(); i++)
		{
			ADIO_PORT *pP = &m_vPort[i];

			str = "addr = " + i2str(pP->m_addr);
			str += " | type = " + i2str(pP->m_type);

			if (pP->bDigital())
			{
				str += " | D | vW = " + i2str(pP->writeD() ? 1 : 0);
				str += " | vR = " + i2str(pP->readD() ? 1 : 0);
			}
			else
			{
				str += " | A | vW = " + f2str(pP->m_vW);
				str += " | vR = " + f2str(pP->readA());
			}

			str += " |";
			pC->addMsg(str);
		}
	}

}
