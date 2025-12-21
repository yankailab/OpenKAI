/*
 *  Created on: June 22, 2020
 *      Author: yankai
 */
#include "_HYMCU_RS485.h"

namespace kai
{

	_HYMCU_RS485::_HYMCU_RS485()
	{
		m_pMB = nullptr;
		m_iSlave = 1;
		m_dpr = 1;
		m_dInit = 20;
		m_cmdInt = 50000;
	}

	_HYMCU_RS485::~_HYMCU_RS485()
	{
	}

	int _HYMCU_RS485::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iSlave", &m_iSlave);
		pK->v("dpr", &m_dpr);
		pK->v("dInit", &m_dInit);
		pK->v("cmdInt", &m_cmdInt);

		Kiss *pKa = pK->child("addr");
		if (pKa)
		{
			pKa->v("setDPR", &m_addr.m_setDPR);
			pKa->v("setDist", &m_addr.m_setDist);
			pKa->v("setDir", &m_addr.m_setDir);
			pKa->v("setSpd", &m_addr.m_setSpd);
			pKa->v("setAcc", &m_addr.m_setAcc);
			pKa->v("setSlaveID", &m_addr.m_setSlaveID);
			pKa->v("setBaudL", &m_addr.m_setBaudL);
			pKa->v("setBaudH", &m_addr.m_setBaudH);

			pKa->v("bComplete", &m_addr.m_bComplete);
			pKa->v("readStat", &m_addr.m_readStat);
			pKa->v("run", &m_addr.m_run);
			pKa->v("stop", &m_addr.m_stop);
			pKa->v("resPos", &m_addr.m_resPos);
		}

		return OK_OK;
	}

	int _HYMCU_RS485::link(void)
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

	int _HYMCU_RS485::start(void)
	{
		NULL__(m_pT, OK_ERR_NOT_FOUND);
		return m_pT->start(getUpdate, this);
	}

	int _HYMCU_RS485::check(void)
	{
		NULL__(m_pMB, OK_ERR_NULLPTR);
		IF__(!m_pMB->bOpen(), OK_ERR_NOT_READY);

		return this->_ActuatorBase::check();
	}

	void _HYMCU_RS485::update(void)
	{
		// setSlaveID(3);
		//  setBaudrate(115200);
		//  saveData();

		while (!initPos())
			;

		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			if (!m_bfSet.b(actuator_move, true))
			{
				while (!stopMove())
					;
				while (!readStatus())
					;
				pos()->setTargetCurrent();
			}

			//		m_pA->m_p.m_vTarget = -m_pA->m_p.m_vTarget;

			updateMove();
		}
	}

	void _HYMCU_RS485::updateMove(void)
	{
		IF_(check() != OK_OK);

		IF_(!bComplete()); // Moving
		IF_(!readStatus());
		IF_(!setPos());
		IF_(!setSpeed());
		IF_(!setAccel());
		move();
	}

	bool _HYMCU_RS485::setDistPerRound(int32_t dpr)
	{
		IF_F(check() != OK_OK);

		uint16_t pB[2];
		pB[0] = HIGH16(dpr);
		pB[1] = LOW16(dpr);
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDPR, 2, pB) != 2);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setPos(void)
	{
		IF_F(check() != OK_OK);

		ACTUATOR_V *pP = pos();
		int32_t step = pP->getDtarget();
		//		int32_t step = m_pA->m_p.m_vTarget;
		IF_F(step == 0);
		int32_t ds = abs(step);

		uint16_t pB[2];
		pB[0] = HIGH16(ds);
		pB[1] = LOW16(ds);
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDist, 2, pB) != 2);
		m_pT->sleepT(m_cmdInt);

		pB[0] = (step > 0) ? 0 : 1;
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDir, 1, pB) != 1);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setSpeed(void)
	{
		IF_F(check() != OK_OK);

		ACTUATOR_V *pS = speed();
		uint16_t b = pS->getTarget();
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setSpd, 1, &b) != 1);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setAccel(void)
	{
		IF_F(check() != OK_OK);

		ACTUATOR_V *pA = accel();
		uint16_t b = pA->getTarget();
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setAcc, 1, &b) != 1);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setBrake(void)
	{
		IF_F(check() != OK_OK);

		return true;
	}

	bool _HYMCU_RS485::setSlaveID(uint16_t iSlave)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setSlaveID, 1, &iSlave) != 1);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setBaudrate(uint32_t baudrate)
	{
		IF_F(check() != OK_OK);

		uint16_t bd;
		bd = baudrate & 0xFFFF;
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setBaudL, 1, &bd) != 1);
		bd = baudrate >> 16;
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setBaudH, 1, &bd) != 1);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::saveData(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_saveData, true) != 1);
		m_pT->sleepT(m_cmdInt);
		return true;
	}

	bool _HYMCU_RS485::move(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_run, true) != 1);
		m_pT->sleepT(m_cmdInt);
		return true;
	}

	bool _HYMCU_RS485::stopMove(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_stop, true) != 1);
		m_pT->sleepT(m_cmdInt);
		return true;
	}

	bool _HYMCU_RS485::resetPos(void)
	{
		IF_F(check() != OK_OK);

		IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_resPos, true) != 1);
		m_pT->sleepT(m_cmdInt);

		ACTUATOR_V *pP = pos();
		pP->set(0);

		return true;
	}

	bool _HYMCU_RS485::initPos(void)
	{
		IF_F(check() != OK_OK);
		IF_F(!setDistPerRound(m_dpr));

		if (m_dInit != 0)
		{
			uint16_t pB[2];
			int32_t ds = abs(m_dInit);
			pB[0] = HIGH16(ds);
			pB[1] = LOW16(ds);
			IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDist, 2, pB) != 2);
			m_pT->sleepT(m_cmdInt);

			pB[0] = (m_dInit > 0) ? 0 : 1;
			IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDir, 1, pB) != 1);
			m_pT->sleepT(m_cmdInt);

			IF_F(!setSpeed());
			IF_F(!setAccel());
			IF_F(!move());

			while (!bComplete())
				;

			pB[0] = (m_dInit > 0) ? 1 : 0;
			IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDir, 1, pB) != 1);
			m_pT->sleepT(m_cmdInt);

			IF_F(!move());

			while (!bComplete())
				;
		}

		while (!resetPos())
			;

		return true;
	}

	bool _HYMCU_RS485::bComplete(void)
	{
		IF_F(check() != OK_OK);

		uint16_t b;
		int r = m_pMB->readRegisters(m_iSlave, m_addr.m_bComplete, 1, &b);
		IF_F(r != 1);
		m_pT->sleepT(m_cmdInt);

		return (b == 1) ? true : false;
	}

	bool _HYMCU_RS485::readStatus(void)
	{
		IF_F(check() != OK_OK);
		IF__(!m_ieReadStatus.updateT(m_pT->getTfrom()), true);

		uint16_t pB[2];
		int r = m_pMB->readRegisters(m_iSlave, m_addr.m_readStat, 2, pB);
		IF_F(r != 2);
		m_pT->sleepT(m_cmdInt);

		ACTUATOR_V *pP = pos();
		//	int p = MAKE32(pB[0], pB[1]);
		int16_t p = pB[1];
		pP->set(p);

		return true;
	}

}
