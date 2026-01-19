/*
 *  Created on: June 22, 2020
 *      Author: yankai
 */
#include "_ZLAC8015D.h"

namespace kai
{

	_ZLAC8015D::_ZLAC8015D()
	{
		m_pMB = nullptr;
		m_ID = 1;
		m_iMode = 3; // speed control
	}

	_ZLAC8015D::~_ZLAC8015D()
	{
	}

	bool _ZLAC8015D::init(const json &j)
	{
		IF_F(!this->_ActuatorBase::init(j));

		jVar(j, "iMode", m_iMode);

		return true;
	}

	bool _ZLAC8015D::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ActuatorBase::link(j, pM));

		string n = "";
		jVar(j, "_Modbus", n);
		m_pMB = (_Modbus *)(pM->findModule(n));
		IF_Le_F(!m_pMB, "_Modbuse not found: " + n);

		return true;
	}

	bool _ZLAC8015D::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _ZLAC8015D::check(void)
	{
		NULL_F(m_pMB);
		IF_F(!m_pMB->bOpen());

		return this->_ActuatorBase::check();
	}

	void _ZLAC8015D::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			// if (!m_bReady)
			// {
			// 	setup();
			// }
			// else
			// {
			// 	readStatus();
			// 	clearAlarm();
			// 	updateMove();
			// }
		}
	}

	void _ZLAC8015D::setup(void)
	{
		IF_(!setMode());
		IF_(!setAccel());
		IF_(!setBrake());
		//		IF_(!power(true));
	}

	// bool _ZLAC8015D::power(bool bON)
	// {
	// 	IF_F(!check());
	// 	IF__(bON == m_bPower, true);

	// 	if (bON)
	// 	{
	// 		IF_F(m_pMB->writeRegister(m_ID, 0x200E, 0x08) != 1);
	// 		m_bPower = true;
	// 	}
	// 	else
	// 	{
	// 		IF_F(m_pMB->writeRegister(m_ID, 0x200E, 0x07) != 1);
	// 		m_bPower = false;
	// 	}

	// 	return true;
	// }

	void _ZLAC8015D::updateMove(void)
	{
		IF_(!check());

		setSpeed();
	}

	bool _ZLAC8015D::setMode(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeRegister(m_ID, 0x200D, m_iMode) != 1);

		return true;
	}

	bool _ZLAC8015D::setAccel(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeRegister(m_ID, 0x2080, (int)m_a.getTarget()) != 1);
		IF_F(m_pMB->writeRegister(m_ID, 0x2081, (int)m_a.getTarget()) != 1);

		return true;
	}

	bool _ZLAC8015D::setBrake(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeRegister(m_ID, 0x2082, (int)m_b.getTarget()) != 1);
		IF_F(m_pMB->writeRegister(m_ID, 0x2083, (int)m_b.getTarget()) != 1);

		return true;
	}

	bool _ZLAC8015D::setSpeed(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeRegister(m_ID, 0x2088, (int)m_s.getTarget()) != 1);
		IF_F(m_pMB->writeRegister(m_ID, 0x2089, (int)m_s.getTarget()) != 1);

		return true;
	}

	bool _ZLAC8015D::stopMove(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeBit(m_ID, 3, true) != 1);
		return true;
	}

	bool _ZLAC8015D::bComplete(void)
	{
		IF_F(!check());

		uint16_t b;
		int r = m_pMB->readRegisters(m_ID, 12, 1, &b);
		IF_F(r != 1);

		return (b == 1) ? true : false;
	}

	bool _ZLAC8015D::readStatus(void)
	{
		IF_F(!check());
		IF__(!m_ieReadStatus.updateT(m_pT->getTfrom()), true);

		uint16_t pB[2];
		int r;
		int16_t p;

		// TODO: to two channels
		r = m_pMB->readRegisters(m_ID, 0x20AB, 1, pB);
		IF_F(r != 1);
		//	int p = MAKE32(pB[0], pB[1]);
		p = pB[0];
		m_p.set(p);

		r = m_pMB->readRegisters(m_ID, 0x20AC, 1, pB);
		IF_F(r != 1);
		p = pB[0];
		m_p.set(p);

		return true;
	}

	bool _ZLAC8015D::clearAlarm(void)
	{
		IF_F(!check());
		IF__(!m_ieReadStatus.updateT(m_pT->getTfrom()), true);

		int r = m_pMB->writeRegister(m_ID, 0x2031, 0x06); // clear alarm
		IF_F(r != 1);

		return true;
	}

}
