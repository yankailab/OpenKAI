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
		m_ID = 1;
		m_ieReadStatus.init(50000);
	}

	_ZLAC8015::~_ZLAC8015()
	{
	}

	bool _ZLAC8015::init(const json &j)
	{
		IF_F(!this->_ActuatorBase::init(j));

		m_iMode = j.value("iMode", 3);
		m_ieReadStatus.m_tInterval = j.value("tIntReadStatus", 50000);

		return true;
	}

	bool _ZLAC8015::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ActuatorBase::link(j, pM));

		string n = j.value("_Modbus", "");
		m_pMB = (_Modbus *)(pM->findModule(n));
		NULL_F(m_pMB);

		return true;
	}

	bool _ZLAC8015::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _ZLAC8015::check(void)
	{
		NULL_F(m_pMB);
		IF_F(!m_pMB->bOpen());

		return this->_ActuatorBase::check();
	}

	void _ZLAC8015::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			if (!m_bfStatus.b(actuator_ready))
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

		m_bfStatus.set(actuator_ready);
	}

	// bool _ZLAC8015::setPower(bool bON)
	// {
	// 	IF_F(!check());
	// 	IF__(bON == m_bPower, true);

	// 	if (bON)
	// 	{
	// 		IF_F(m_pMB->writeRegister(m_ID, 0x2031, 0x08) != 1);
	// 		m_bPower = true;
	// 	}
	// 	else
	// 	{
	// 		IF_F(m_pMB->writeRegister(m_ID, 0x2031, 0x07) != 1);
	// 		m_bPower = false;
	// 	}

	// 	return true;
	// }

	void _ZLAC8015::updateMove(void)
	{
		IF_(!check());

		setSpeed();
	}

	bool _ZLAC8015::setMode(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeRegister(m_ID, 0x2032, m_iMode) != 1);

		return true;
	}

	bool _ZLAC8015::setAccel(void)
	{
		IF_F(!check());

		uint16_t v = m_a.getTarget();
		IF_F(m_pMB->writeRegister(m_ID, 0x2037, v) != 1);

		return true;
	}

	bool _ZLAC8015::setBrake(void)
	{
		IF_F(!check());

		uint16_t v = m_b.getTarget();
		IF_F(m_pMB->writeRegister(m_ID, 0x2038, v) != 1);

		return true;
	}

	bool _ZLAC8015::setSpeed(void)
	{
		IF_F(!check());

		int16_t v = m_s.getTarget();
		IF_F(m_pMB->writeRegister(m_ID, 0x203A, v) != 1);

		return true;
	}

	bool _ZLAC8015::stopMove(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeBit(m_ID, 3, true) != 1);
		return true;
	}

	bool _ZLAC8015::bComplete(void)
	{
		IF_F(!check());

		uint16_t b;
		int r = m_pMB->readRegisters(m_ID, 12, 1, &b);
		IF_F(r != 1);

		return (b == 1) ? true : false;
	}

	bool _ZLAC8015::readStatus(void)
	{
		IF_F(!check());
		IF__(!m_ieReadStatus.updateT(m_pT->getTfrom()), true);

		uint16_t pB[2];
		int r = m_pMB->readRegisters(m_ID, 0x202C, 1, pB);
		IF_F(r != 1);

		//	int p = MAKE32(pB[0], pB[1]);
		int16_t p = pB[0];

		m_p.set(p);

		return true;
	}

	bool _ZLAC8015::clearAlarm(void)
	{
		IF_F(!check());
		IF__(!m_ieReadStatus.updateT(m_pT->getTfrom()), true);

		int r = m_pMB->writeRegister(m_ID, 0x2031, 0x06); // clear alarm
		IF_F(r != 1);

		return true;
	}

}
