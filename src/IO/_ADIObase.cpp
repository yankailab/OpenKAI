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

	bool _ADIObase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		//		pK->v("nFIFO", &m_nFIFO);

		Kiss *pKp = pK->child("port");
		IF_T(pKp->empty());

		int i = 0;
		while (1)
		{
			IF_F(i >= ADIO_MAX_PORT);
			Kiss *pP = pKp->child(i++);
			if (pP->empty())
				break;

			ADIO_PORT port;
			port.clear();
			pP->v("bDigital", &port.m_bDigital);
			pP->v("vW", &port.m_vW);
			pP->v("vR", &port.m_vR);

			m_vPort.push_back(port);
		}

		return true;
	}

	bool _ADIObase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

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
		string str = "| ";

		for (int i = 0; i < m_vPort.size(); i++)
		{
			ADIO_PORT *pP = &m_vPort[i];

			if (pP->m_bDigital)
				str += i2str(pP->readD() ? 1 : 0);
			else
				str += f2str(pP->readA());

			str += " | ";
		}

		pC->addMsg(str);
	}

}
