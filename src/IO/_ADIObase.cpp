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
		m_status = adio_unknown;
	}

	_ADIObase::~_ADIObase()
	{
	}

	int _ADIObase::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		Kiss *pKp = pK->child("port");
		IF__(pKp->empty(), OK_OK);

		int i = 0;
		while (1)
		{
			IF__(i >= ADIO_MAX_PORT, OK_ERR_INVALID_VALUE);
			Kiss *pP = pKp->child(i++);
			if (pP->empty())
				break;

			ADIO_PORT port;
			port.clear();
			pP->v("bDigital", &port.m_bDigital);
			pP->v("type", (int*)&port.m_type);
			pP->v("addr", &port.m_addr);
			pP->v("vW", &port.m_vW);
			pP->v("vR", &port.m_vR);

			m_vPort.push_back(port);
		}

		return true;
	}

	int _ADIObase::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));

		return true;
	}

	bool _ADIObase::open(void)
	{
		return false;
	}

	bool _ADIObase::bOpen(void)
	{
		return (m_status == adio_opened);
	}

	void _ADIObase::close(void)
	{
		m_status = adio_closed;
	}

	void _ADIObase::updateW(void)
	{
	}

	void _ADIObase::updateR(void)
	{
	}

	bool _ADIObase::writeD(int iPort, bool b)
	{
		IF_F(iPort >= m_vPort.size());

		return m_vPort[iPort].writeD(b);
	}

	bool _ADIObase::writeA(int iPort, float v)
	{
		IF_F(iPort >= m_vPort.size());

		return m_vPort[iPort].writeA(v);
	}

	bool _ADIObase::readD(int iPort)
	{
		IF_F(iPort >= m_vPort.size());

		return m_vPort[iPort].readD();
	}

	float _ADIObase::readA(int iPort)
	{
		IF_F(iPort >= m_vPort.size());

		return m_vPort[iPort].readA();
	}

	void _ADIObase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = ((_Console *)pConsole);
		string str;

		for (int i = 0; i < m_vPort.size(); i++)
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
