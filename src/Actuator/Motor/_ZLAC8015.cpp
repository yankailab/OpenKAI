/*
 *  Created on: June 22, 2020
 *      Author: yankai
 */
#include "_ZLAC8015.h"

namespace kai
{

	_ZLAC8015::_ZLAC8015()
	{
		m_pMB = nullptr;
		m_iSlave = 1;
		m_iMode = 3; //speed control

		m_ieReadStatus.init(50000);
	}

	_ZLAC8015::~_ZLAC8015()
	{
	}

	int _ZLAC8015::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iSlave", &m_iSlave);
		pK->v("iMode", &m_iMode);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

		return OK_OK;
	}

	int _ZLAC8015::link(void)
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

	int _ZLAC8015::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _ZLAC8015::check(void)
	{
		NULL__(m_pMB, OK_ERR_NULLPTR);
		IF__(!m_pMB->bOpen(), OK_ERR_NOT_READY);

		return this->_ActuatorBase::check();
	}

	void _ZLAC8015::update(void)
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
				readStatus();
				clearAlarm();
				updateMove();
			}
		}
	}

	void _ZLAC8015::setup(void)
	{
		IF_(!setMode());
		IF_(!setAccel());
		IF_(!setBrake());
//		IF_(!setPower(true));

		ACTUATOR_CHAN* pChan = getChan();
		pChan->m_bfStatus.set(actuator_ready);
	}

	// bool _ZLAC8015::setPower(bool bON)
	// {
	// 	IF_F(check() != OK_OK);
	// 	IF__(bON == m_bPower, true);

	// 	if (bON)
	// 	{
	// 		IF_F(m_pMB->writeRegister(m_iSlave, 0x2031, 0x08) != 1);
	// 		m_bPower = true;
	// 	}
	// 	else
	// 	{
	// 		IF_F(m_pMB->writeRegister(m_iSlave, 0x2031, 0x07) != 1);
	// 		m_bPower = false;
	// 	}

	// 	return true;
	// }

	void _ZLAC8015::updateMove(void)
	{
		IF_(check() != OK_OK);

		setSpeed();
	}

	bool _ZLAC8015::setMode(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2032, m_iMode) != 1);

		return true;
	}

	bool _ZLAC8015::setAccel(void)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan();

		uint16_t v = pChan->accel()->getTarget();
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2037, v) != 1);

		return true;
	}

	bool _ZLAC8015::setBrake(void)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan();

		uint16_t v = pChan->brake()->getTarget();
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2038, v) != 1);

		return true;
	}

	bool _ZLAC8015::setSpeed(void)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan();

		int16_t v = pChan->speed()->getTarget();
		IF_F(m_pMB->writeRegister(m_iSlave, 0x203A, v) != 1);

		return true;
	}

	bool _ZLAC8015::stopMove(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeBit(m_iSlave, 3, true) != 1);
		return true;
	}

	bool _ZLAC8015::bComplete(void)
	{
		IF_F(check() != OK_OK);

		uint16_t b;
		int r = m_pMB->readRegisters(m_iSlave, 12, 1, &b);
		IF_F(r != 1);

		return (b == 1) ? true : false;
	}

	bool _ZLAC8015::readStatus(void)
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

	bool _ZLAC8015::clearAlarm(void)
	{
		IF_F(check() != OK_OK);
		IF__(!m_ieReadStatus.update(m_pT->getTfrom()), true);

		int r = m_pMB->writeRegister(m_iSlave, 0x2031, 0x06); //clear alarm
		IF_F(r != 1);

		return true;
	}

}
