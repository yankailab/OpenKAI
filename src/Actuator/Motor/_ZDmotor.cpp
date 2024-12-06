/*
 *  Created on: Sept 21, 2022
 *      Author: yankai
 */
#include "_ZDmotor.h"

namespace kai
{

	_ZDmotor::_ZDmotor()
	{
		m_pMB = nullptr;
		m_iSlave = 1;
		m_iMode = 3; // speed control

		m_ieReadStatus.init(50000);
	}

	_ZDmotor::~_ZDmotor()
	{
	}

	int _ZDmotor::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iSlave", &m_iSlave);
		pK->v("iMode", &m_iMode);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

		return OK_OK;
	}

	int _ZDmotor::link(void)
	{
		CHECK_(this->_ActuatorBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		IF__(!pK->v("_Modbus", &n), OK_ERR_NOT_FOUND);
		m_pMB = (_Modbus *)(pK->findModule(n));
		NULL__(m_pMB, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _ZDmotor::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _ZDmotor::check(void)
	{
		NULL__(m_pMB, OK_ERR_NULLPTR);
		IF__(!m_pMB->bOpen(), OK_ERR_NOT_READY);

		return this->_ActuatorBase::check();
	}

	void _ZDmotor::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			ACTUATOR_CHAN* pChan = getChan();

			if (!pChan->m_bfStatus.b(actuator_ready))
			{
				setup();
			}
			else
			{
				//				readStatus();
				clearAlarm();
				updateMove();
			}
		}
	}

	void _ZDmotor::setup(void)
	{
		ACTUATOR_CHAN* pChan = getChan();

		// IF_(!setMode());
		// IF_(!setAccel());
		// IF_(!setBrake());
		// IF_(!power(true));

		pChan->m_bfStatus.set(actuator_ready);
	}

	bool _ZDmotor::setPower(bool bON)
	{
		// IF_F(check() != OK_OK);
		// IF__(bON == m_bPower, true);

		// if (bON)
		// {
		// 	IF_F(m_pMB->writeRegister(m_iSlave, 0x2031, 0x08) != 1);
		// 	m_bPower = true;
		// }
		// else
		// {
		// 	IF_F(m_pMB->writeRegister(m_iSlave, 0x2031, 0x07) != 1);
		// 	m_bPower = false;
		// }

		return true;
	}

	void _ZDmotor::updateMove(void)
	{
		IF_(check() != OK_OK);

		setSpeed();
	}

	bool _ZDmotor::setMode(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2032, m_iMode) != 1);

		return true;
	}

	bool _ZDmotor::setAccel(void)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan();

		uint16_t v = pChan->accel()->getTarget();
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2037, v) != 1);

		return true;
	}

	bool _ZDmotor::setBrake(void)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan();

		uint16_t v = pChan->brake()->getTarget();
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2038, v) != 1);

		return true;
	}

	bool _ZDmotor::setSpeed(void)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan();

		uint16_t v = pChan->speed()->getTarget();
		int16_t d = (v >= 0) ? 1 : 2;
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2000, d) != 1);
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2001, abs(v)) != 1);

		return true;
	}

	bool _ZDmotor::stopMove(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeBit(m_iSlave, 3, true) != 1);
		return true;
	}

	bool _ZDmotor::bComplete(void)
	{
		IF_F(check() != OK_OK);

		uint16_t b;
		int r = m_pMB->readRegisters(m_iSlave, 12, 1, &b);
		IF_F(r != 1);

		return (b == 1) ? true : false;
	}

	bool _ZDmotor::readStatus(void)
	{
		IF_F(check() != OK_OK);
		IF__(!m_ieReadStatus.update(m_pT->getTfrom()), true);

		uint16_t pB[2];
		int r = m_pMB->readRegisters(m_iSlave, 0x202C, 1, pB);
		IF_F(r != 1);

		//	int p = MAKE32(pB[0], pB[1]);
		int16_t p = pB[0];
		ACTUATOR_CHAN* pChan = getChan();
		pChan->pos()->set(p);

		return true;
	}

	bool _ZDmotor::clearAlarm(void)
	{
		IF_F(check() != OK_OK);
		//		IF__(!m_ieReadStatus.update(m_pT->getTfrom()));

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2000, 0x07) != 1);

		return true;
	}

}
