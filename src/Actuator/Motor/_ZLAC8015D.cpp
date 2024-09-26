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
		m_pL = nullptr;
		m_pR = nullptr;

		m_ieReadStatus.init(50000);
	}

	_ZLAC8015D::~_ZLAC8015D()
	{
	}

	int _ZLAC8015D::init(void *pKiss)
	{
		CHECK_(this->_MultiActuatorsBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iSlave", &m_iSlave);
		pK->v("iMode", &m_iMode);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

		return OK_OK;
	}

	int _ZLAC8015D::link(void)
	{
		CHECK_(this->_MultiActuatorsBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		IF__(m_nAB != 2, OK_ERR_INVALID_VALUE);

		m_pL = m_pAB[0];
		m_pR = m_pAB[1];

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
		NULL__(m_pL, OK_ERR_NULLPTR);
		NULL__(m_pR, OK_ERR_NULLPTR);

		return this->_MultiActuatorsBase::check();
	}

	void _ZLAC8015D::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if (!m_bReady)
			{
				setup();
			}
			else
			{
				readStatus();
				clearAlarm();
				updateMove();
			}

			m_pT->autoFPSto();
		}
	}

	void _ZLAC8015D::setup(void)
	{
		IF_(!setMode());
		IF_(!setAccel());
		IF_(!setBrake());
		IF_(!power(true));

		m_bReady = true;
	}

	bool _ZLAC8015D::power(bool bON)
	{
		IF_F(check() != OK_OK);
		IF__(bON == m_bPower, true);

		if (bON)
		{
			IF_F(m_pMB->writeRegister(m_iSlave, 0x200E, 0x08) != 1);
			m_bPower = true;
		}
		else
		{
			IF_F(m_pMB->writeRegister(m_iSlave, 0x200E, 0x07) != 1);
			m_bPower = false;
		}

		return true;
	}

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

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2080, (int)m_pL->getAtarget()) != 1);
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2081, (int)m_pR->getAtarget()) != 1);

		return true;
	}

	bool _ZLAC8015D::setBrake(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2082, (int)m_pL->getBtarget()) != 1);
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2083, (int)m_pR->getBtarget()) != 1);

		return true;
	}

	bool _ZLAC8015D::setSpeed(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegister(m_iSlave, 0x2088, (int)m_pL->getStarget()) != 1);
		IF_F(m_pMB->writeRegister(m_iSlave, 0x2089, (int)m_pR->getStarget()) != 1);

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
		m_pL->setP(p);

		r = m_pMB->readRegisters(m_iSlave, 0x20AC, 1, pB);
		IF_F(r != 1);
		p = pB[0];
		m_pR->setP(p);

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
