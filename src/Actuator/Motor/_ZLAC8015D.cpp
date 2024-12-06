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
		m_iSlave = 1;
		m_iMode = 3; //speed control

		m_ieReadStatus.init(50000);
	}

	_ZLAC8015D::~_ZLAC8015D()
	{
	}

	int _ZLAC8015D::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		IF__(m_vChan.size() != 2, OK_ERR_INVALID_VALUE);

		pK->v("iSlave", &m_iSlave);
		pK->v("iMode", &m_iMode);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

		return OK_OK;
	}

	int _ZLAC8015D::link(void)
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

	int _ZLAC8015D::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _ZLAC8015D::check(void)
	{
		NULL__(m_pMB, OK_ERR_NULLPTR);
		IF__(!m_pMB->bOpen(), OK_ERR_NOT_READY);

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
	// 	IF_F(check() != OK_OK);
	// 	IF__(bON == m_bPower, true);

	// 	if (bON)
	// 	{
	// 		IF_F(m_pMB->writeRegister(m_iSlave, 0x200E, 0x08) != 1);
	// 		m_bPower = true;
	// 	}
	// 	else
	// 	{
	// 		IF_F(m_pMB->writeRegister(m_iSlave, 0x200E, 0x07) != 1);
	// 		m_bPower = false;
	// 	}

	// 	return true;
	// }

	void _ZLAC8015D::updateMove(void)
	{
		IF_(check() != OK_OK);

		setSpeed();
	}

	bool _ZLAC8015D::setMode(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x200D, m_iMode) != 1);

		return true;
	}

	bool _ZLAC8015D::setAccel(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2080, (int)getChan(0)->accel()->getTarget()) != 1);
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2081, (int)getChan(1)->accel()->getTarget()) != 1);

		return true;
	}

	bool _ZLAC8015D::setBrake(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2082, (int)getChan(0)->brake()->getTarget()) != 1);
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2083, (int)getChan(1)->brake()->getTarget()) != 1);

		return true;
	}

	bool _ZLAC8015D::setSpeed(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2088, (int)getChan(0)->speed()->getTarget()) != 1);
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2089, (int)getChan(1)->speed()->getTarget()) != 1);

		return true;
	}

	bool _ZLAC8015D::stopMove(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeBit(m_iSlave, 3, true) != 1);
		return true;
	}

	bool _ZLAC8015D::bComplete(void)
	{
		IF_F(check() != OK_OK);

		uint16_t b;
		int r = m_pMB->readRegisters(m_iSlave, 12, 1, &b);
		IF_F(r != 1);

		return (b == 1) ? true : false;
	}

	bool _ZLAC8015D::readStatus(void)
	{
		IF_F(check() != OK_OK);
		IF__(!m_ieReadStatus.update(m_pT->getTfrom()), true);

		uint16_t pB[2];
		int r;
		int16_t p;

		r = m_pMB->readRegisters(m_iSlave, 0x20AB, 1, pB);
		IF_F(r != 1);
		//	int p = MAKE32(pB[0], pB[1]);
		p = pB[0];
		getChan(0)->pos()->set(p);

		r = m_pMB->readRegisters(m_iSlave, 0x20AC, 1, pB);
		IF_F(r != 1);
		p = pB[0];
		getChan(1)->pos()->set(p);

		return true;
	}

	bool _ZLAC8015D::clearAlarm(void)
	{
		IF_F(check() != OK_OK);
		IF__(!m_ieReadStatus.update(m_pT->getTfrom()), true);

		int r = m_pMB->writeRegister(m_iSlave, 0x2031, 0x06); //clear alarm
		IF_F(r != 1);

		return true;
	}

}
