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
		m_ID = 1;
		m_iMode = 3; // speed control
	}

	_ZDmotor::~_ZDmotor()
	{
	}

	bool _ZDmotor::init(const json &j)
	{
		IF_F(!this->_ActuatorBase::init(j));

		jVar(j, "iMode", m_iMode);

		return true;
	}

	bool _ZDmotor::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ActuatorBase::link(j, pM));

		string n = "";
		jVar(j, "_Modbus", n);
		m_pMB = (_Modbus *)(pM->findModule(n));
		IF_Le_F(!m_pMB, "_Modbuse not found: " + n);

		return true;
	}

	bool _ZDmotor::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _ZDmotor::check(void)
	{
		NULL_F(m_pMB);
		IF_F(!m_pMB->bOpen());

		return this->_ActuatorBase::check();
	}

	void _ZDmotor::update(void)
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
				//				readStatus();
				clearAlarm();
				updateMove();
			}
		}
	}

	void _ZDmotor::setup(void)
	{
		// IF_(!setMode());
		// IF_(!setAccel());
		// IF_(!setBrake());
		// IF_(!power(true));

		m_bfStatus.set(actuator_ready);
	}

	bool _ZDmotor::setPower(bool bON)
	{
		// IF_F(!check());
		// IF__(bON == m_bPower, true);

		// if (bON)
		// {
		// 	IF_F(m_pMB->writeRegister(m_ID, 0x2031, 0x08) != 1);
		// 	m_bPower = true;
		// }
		// else
		// {
		// 	IF_F(m_pMB->writeRegister(m_ID, 0x2031, 0x07) != 1);
		// 	m_bPower = false;
		// }

		return true;
	}

	void _ZDmotor::updateMove(void)
	{
		IF_(!check());

		setSpeed();
	}

	bool _ZDmotor::setMode(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeRegister(m_ID, 0x2032, m_iMode) != 1);

		return true;
	}

	bool _ZDmotor::setAccel(void)
	{
		IF_F(!check());

		uint16_t v = m_a.getTarget();
		IF_F(m_pMB->writeRegister(m_ID, 0x2037, v) != 1);

		return true;
	}

	bool _ZDmotor::setBrake(void)
	{
		IF_F(!check());

		uint16_t v = m_b.getTarget();
		IF_F(m_pMB->writeRegister(m_ID, 0x2038, v) != 1);

		return true;
	}

	bool _ZDmotor::setSpeed(void)
	{
		IF_F(!check());

		uint16_t v = m_s.getTarget();
		int16_t d = (v >= 0) ? 1 : 2;
		IF_F(m_pMB->writeRegister(m_ID, 0x2000, d) != 1);
		IF_F(m_pMB->writeRegister(m_ID, 0x2001, abs(v)) != 1);

		return true;
	}

	bool _ZDmotor::stopMove(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeBit(m_ID, 3, true) != 1);
		return true;
	}

	bool _ZDmotor::bComplete(void)
	{
		IF_F(!check());

		uint16_t b;
		int r = m_pMB->readRegisters(m_ID, 12, 1, &b);
		IF_F(r != 1);

		return (b == 1) ? true : false;
	}

	bool _ZDmotor::readStatus(void)
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

	bool _ZDmotor::clearAlarm(void)
	{
		IF_F(!check());
		//		IF__(!m_ieReadStatus.update(m_pT->getTfrom()));

		IF_F(m_pMB->writeRegister(m_ID, 0x2000, 0x07) != 1);

		return true;
	}

}
