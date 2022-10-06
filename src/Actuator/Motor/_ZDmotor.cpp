/*
 *  Created on: Sept 21, 2022
 *      Author: yankai
 */
#include "_ZDmotor.h"

namespace kai
{

	_ZDmotor::_ZDmotor()
	{
		m_pMB = NULL;
		m_iSlave = 1;
		m_iMode = 3; //speed control
		m_pA = NULL;

		m_ieReadStatus.init(50000);
	}

	_ZDmotor::~_ZDmotor()
	{
	}

	bool _ZDmotor::init(void *pKiss)
	{
		IF_F(!this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iSlave", &m_iSlave);
		pK->v("iMode", &m_iMode);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

		m_pA = &m_vAxis[0];

		string n;
		n = "";
		F_ERROR_F(pK->v("_Modbus", &n));
		m_pMB = (_Modbus *)(pK->getInst(n));
		IF_Fl(!m_pMB, n + " not found");

		return true;
	}

	bool _ZDmotor::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ZDmotor::check(void)
	{
		NULL__(m_pMB, -1);
		IF__(!m_pMB->bOpen(), -1);
		NULL__(m_pA, -1);

		return this->_ActuatorBase::check();
	}

	void _ZDmotor::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (!m_bReady)
			{
				setup();
			}
			else
			{
				//				readStatus();
				//				clearAlarm();
				updateMove();
			}

			m_pT->autoFPSto();
		}
	}

	void _ZDmotor::setup(void)
	{
		// IF_(!setMode());
		// IF_(!setAccel());
		// IF_(!setBrake());
		// IF_(!power(true));

		m_bReady = true;
	}

	bool _ZDmotor::power(bool bON)
	{
		IF_F(check() < 0);
		IF_T(bON == m_bPower);

		if (bON)
		{
			IF_F(m_pMB->writeRegister(m_iSlave, 0x2031, 0x08) != 1);
			m_bPower = true;
		}
		else
		{
			IF_F(m_pMB->writeRegister(m_iSlave, 0x2031, 0x07) != 1);
			m_bPower = false;
		}

		return true;
	}

	void _ZDmotor::updateMove(void)
	{
		IF_(check() < 0);

		setSpeed();
	}

	bool _ZDmotor::setMode(void)
	{
		IF_F(check() < 0);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2032, m_iMode) != 1);

		return true;
	}

	bool _ZDmotor::setAccel(void)
	{
		IF_F(check() < 0);

		uint16_t v = m_pA->m_a.m_vTarget;
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2037, v) != 1);

		return true;
	}

	bool _ZDmotor::setBrake(void)
	{
		IF_F(check() < 0);

		uint16_t v = m_pA->m_b.m_vTarget;
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2038, v) != 1);

		return true;
	}

	bool _ZDmotor::setSpeed(void)
	{
		IF_F(check() < 0);

		int16_t v = m_pA->m_s.m_vTarget;
		int16_t d = (v >= 0) ? 1 : 2;
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2000, d) != 1);
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2001, abs(v)) != 1);

		return true;
	}

	bool _ZDmotor::stop(void)
	{
		IF_F(check() < 0);

		IF_F(m_pMB->writeBit(m_iSlave, 3, true) != 1);
		return true;
	}

	bool _ZDmotor::bComplete(void)
	{
		IF_F(check() < 0);

		uint16_t b;
		int r = m_pMB->readRegisters(m_iSlave, 12, 1, &b);
		IF_F(r != 1);

		return (b == 1) ? true : false;
	}

	bool _ZDmotor::readStatus(void)
	{
		IF_F(check() < 0);
		IF_T(!m_ieReadStatus.update(m_pT->getTfrom()));

		uint16_t pB[2];
		int r = m_pMB->readRegisters(m_iSlave, 0x202C, 1, pB);
		IF_F(r != 1);

		//	int p = MAKE32(pB[0], pB[1]);
		int16_t p = pB[0];
		m_pA->m_p.m_v = p;

		return true;
	}

	bool _ZDmotor::clearAlarm(void)
	{
		IF_F(check() < 0);
		IF_T(!m_ieReadStatus.update(m_pT->getTfrom()));

		int r = m_pMB->writeRegister(m_iSlave, 0x2031, 0x06); //clear alarm
		IF_F(r != 1);

		return true;
	}

}
