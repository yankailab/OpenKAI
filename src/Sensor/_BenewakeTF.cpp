/*
 *  Created on: July 24, 2021
 *      Author: yankai
 */
#include "_BenewakeTF.h"

namespace kai
{

	_BenewakeTF::_BenewakeTF()
	{
		m_pIO = NULL;
		m_nDiv = 1;
	}

	_BenewakeTF::~_BenewakeTF()
	{
	}

	bool _BenewakeTF::init(void *pKiss)
	{
		IF_F(!this->_DistSensorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;

		n = "";
		F_ERROR_F(pK->v("_IOBase", &n));
		m_pIO = (_IOBase *)(pK->getInst(n));
		IF_Fl(!m_pIO, n + " not found");

		m_frame.init(9);
		m_bReady = true;

		return true;
	}

	bool _BenewakeTF::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _BenewakeTF::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_pIO)
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			if (!m_pIO->isOpen())
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			m_pT->autoFPSfrom();

			while (readCMD())
			{
				handleCMD();
			}

			m_pT->autoFPSto();
		}
	}

	bool _BenewakeTF::readCMD(void)
	{
		uint8_t b;
		int nB;

		while ((nB = m_pIO->read(&b, 1)) > 0)
		{
			if (m_frame.m_header != 0)
			{
				m_frame.m_pB[m_frame.m_iB] = b;
				m_frame.m_iB++;

				if (m_frame.m_iB >= 9)
				{
					return true;
				}
			}
			else if (b == BENEWAKE_BEGIN)
			{
				m_frame.m_header = b;
				m_frame.m_pB[0] = b;
				m_frame.m_iB = 1;
			}
		}

		return false;
	}

	void _BenewakeTF::handleCMD(void)
	{
		uint8_t cs = verifyCheckSum(m_frame.m_pB, 9);
//		IF_(!cs);
		uint16_t d = m_frame.m_pB[2];
		d |= m_frame.m_pB[3] << 8;
		float df = d * 0.01;

		this->input(0,
					df,
					0);
		m_frame.reset();
	}

	uint8_t _BenewakeTF::verifyCheckSum(const uint8_t *data, size_t dataLength)
	{
		uint8_t sum = 0;
		for (size_t i = 0; i < dataLength - 1; i++)
		{
			sum += data[i];
		}
		return sum == data[dataLength - 1];
	}

	DIST_SENSOR_TYPE _BenewakeTF::type(void)
	{
		return ds_BenewakeTF;
	}

	void _BenewakeTF::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		msg += "nDiv=" + i2str(m_nDiv);
		pC->addMsg(msg);

		int i;
		msg = "";
		for (i = 0; i < m_nDiv; i++)
		{
			DIST_SENSOR_DIV *pD = &m_pDiv[i];
			msg += " | " + f2str(pD->d()) + " (" + f2str(pD->a()) + ")";
		}
		msg += " |";
		pC->addMsg(msg);
#endif
	}

}
