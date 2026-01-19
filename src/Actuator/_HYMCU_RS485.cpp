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

	bool _HYMCU_RS485::init(const json &j)
	{
		IF_F(!this->_ActuatorBase::init(j));

		jVar(j, "iSlave", m_iSlave);
		jVar(j, "dpr", m_dpr);
		jVar(j, "dInit", m_dInit);
		jVar(j, "cmdInt", m_cmdInt);

		const json &ja = j.at("addr");
		if (ja.is_object())
		{
			jVar(ja, "setDPR", m_addr.m_setDPR);
			jVar(ja, "setDist", m_addr.m_setDist);
			jVar(ja, "setDir", m_addr.m_setDir);
			jVar(ja, "setSpd", m_addr.m_setSpd);
			jVar(ja, "setAcc", m_addr.m_setAcc);
			jVar(ja, "setSlaveID", m_addr.m_setSlaveID);
			jVar(ja, "setBaudL", m_addr.m_setBaudL);
			jVar(ja, "setBaudH", m_addr.m_setBaudH);

			jVar(ja, "bComplete", m_addr.m_bComplete);
			jVar(ja, "readStat", m_addr.m_readStat);
			jVar(ja, "run", m_addr.m_run);
			jVar(ja, "stop", m_addr.m_stop);
			jVar(ja, "resPos", m_addr.m_resPos);
			jVar(ja, "saveData", m_addr.m_saveData);
		}

		return true;
	}

	bool _HYMCU_RS485::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ActuatorBase::link(j, pM));

		string n = "";
		jVar(j, "_Modbus", n);
		m_pMB = (_Modbus *)(pM->findModule(n));
		IF_Le_F(!m_pMB, "_Modbus not found: " + n);

		return true;
	}

	bool _HYMCU_RS485::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _HYMCU_RS485::check(void)
	{
		NULL_F(m_pMB);
		IF_F(!m_pMB->bOpen());

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
		IF_(!check());

		IF_(!bComplete()); // Moving
		IF_(!readStatus());
		IF_(!setPos());
		IF_(!setSpeed());
		IF_(!setAccel());
		move();
	}

	bool _HYMCU_RS485::setDistPerRound(int32_t dpr)
	{
		IF_F(!check());

		uint16_t pB[2];
		pB[0] = HIGH16(dpr);
		pB[1] = LOW16(dpr);
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDPR, 2, pB) != 2);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setPos(void)
	{
		IF_F(!check());

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
		IF_F(!check());

		ACTUATOR_V *pS = speed();
		uint16_t b = pS->getTarget();
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setSpd, 1, &b) != 1);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setAccel(void)
	{
		IF_F(!check());

		ACTUATOR_V *pA = accel();
		uint16_t b = pA->getTarget();
		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setAcc, 1, &b) != 1);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setBrake(void)
	{
		IF_F(!check());

		return true;
	}

	bool _HYMCU_RS485::setSlaveID(uint16_t iSlave)
	{
		IF_F(!check());

		IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setSlaveID, 1, &iSlave) != 1);
		m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _HYMCU_RS485::setBaudrate(uint32_t baudrate)
	{
		IF_F(!check());

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
		IF_F(!check());

		IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_saveData, true) != 1);
		m_pT->sleepT(m_cmdInt);
		return true;
	}

	bool _HYMCU_RS485::move(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_run, true) != 1);
		m_pT->sleepT(m_cmdInt);
		return true;
	}

	bool _HYMCU_RS485::stopMove(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_stop, true) != 1);
		m_pT->sleepT(m_cmdInt);
		return true;
	}

	bool _HYMCU_RS485::resetPos(void)
	{
		IF_F(!check());

		IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_resPos, true) != 1);
		m_pT->sleepT(m_cmdInt);

		ACTUATOR_V *pP = pos();
		pP->set(0);

		return true;
	}

	bool _HYMCU_RS485::initPos(void)
	{
		IF_F(!check());
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
		IF_F(!check());

		uint16_t b;
		int r = m_pMB->readRegisters(m_iSlave, m_addr.m_bComplete, 1, &b);
		IF_F(r != 1);
		m_pT->sleepT(m_cmdInt);

		return (b == 1) ? true : false;
	}

	bool _HYMCU_RS485::readStatus(void)
	{
		IF_F(!check());
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
