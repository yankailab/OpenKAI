/*
 *  Created on: Jan 6, 2024
 *      Author: yankai
 */
#include "_FeeTech.h"

namespace kai
{

	_FeeTech::_FeeTech()
	{
		m_pIO = NULL;
		m_ID = 1;
		m_pA = NULL;

		m_ieReadStatus.init(50000);
	}

	_FeeTech::~_FeeTech()
	{
	}

	bool _FeeTech::init(void *pKiss)
	{
		IF_F(!this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iID", &m_ID);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

		Kiss *pKa = pK->child("addr");
		if (pKa)
		{
			pKa->v("ID", &m_addr.m_ID);
			pKa->v("baudrate", &m_addr.m_baudrate);
			pKa->v("angleMin", &m_addr.m_angleMin);
			pKa->v("angleMax", &m_addr.m_angleMax);
			pKa->v("bTorque", &m_addr.m_bTorque);
			pKa->v("accel", &m_addr.m_accel);
			pKa->v("posTarget", &m_addr.m_posTarget);
			pKa->v("spdTarget", &m_addr.m_spdTarget);
			pKa->v("bLock", &m_addr.m_bLock);
			pKa->v("pos", &m_addr.m_pos);
			pKa->v("spd", &m_addr.m_spd);
			pKa->v("load", &m_addr.m_load);
			pKa->v("voltage", &m_addr.m_voltage);
			pKa->v("temprature", &m_addr.m_temprature);
			pKa->v("current", &m_addr.m_current);
		}

		m_pA = &m_vAxis[0];

		return true;
	}

	bool _FeeTech::link(void)
	{
		IF_F(!this->_ActuatorBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		F_ERROR_F(pK->v("_IObase", &n));
		m_pIO = (_IObase *)(pK->getInst(n));
		IF_Fl(!m_pIO, n + " not found");

		return true;
	}

	bool _FeeTech::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _FeeTech::check(void)
	{
		NULL__(m_pIO, -1);
		IF__(!m_pIO->bOpen(), -1);
		NULL__(m_pA, -1);

		return this->_ActuatorBase::check();
	}

	void _FeeTech::update(void)
	{
		// setID(3);
		//  setBaudrate(115200);
		//  saveData();

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (m_bf.b(ACT_BF_STOP, true))
			{
				while (!readStatus())
					;
				m_pA->m_p.m_vTarget = m_pA->m_p.m_v;
			}

			//		m_pA->m_p.m_vTarget = -m_pA->m_p.m_vTarget;

			updateMove();

			m_pT->autoFPSto();
		}
	}

	void _FeeTech::updateMove(void)
	{
		IF_(check() < 0);

		IF_(!readStatus());
		IF_(!setPos());
		IF_(!setSpeed());
		IF_(!setAccel());
	}

	bool _FeeTech::setID(uint16_t iSlave)
	{
		IF_F(check() < 0);

		//		IF_F(m_pIO->writeRegisters(m_ID, m_addr.m_setSlaveID, 1, &iSlave) != 1);

		return true;
	}

	bool _FeeTech::setBaudrate(uint32_t baudrate)
	{
		IF_F(check() < 0);

		// uint16_t bd;
		// bd = baudrate & 0xFFFF;
		// IF_F(m_pIO->writeRegisters(m_ID, m_addr.m_setBaudL, 1, &bd) != 1);
		// bd = baudrate >> 16;
		// IF_F(m_pIO->writeRegisters(m_ID, m_addr.m_setBaudH, 1, &bd) != 1);
		// m_pT->sleepT(m_cmdInt);

		return true;
	}

	bool _FeeTech::saveData(void)
	{
		IF_F(check() < 0);

		return true;
	}

	bool _FeeTech::setPos(void)
	{
		IF_F(check() < 0);

		int16_t p = m_pA->m_p.m_vTarget;

		uint8_t pCmd[FEETECH_HEADER + 3];
		pCmd[0] = 0xFF;
		pCmd[1] = 0xFF;
		pCmd[2] = m_ID;
		pCmd[3] = 5;
		pCmd[4] = 0x03;
		pCmd[5] = 0x2A;
		// pCmd[6] = ;
		// pCmd[7] = ;
		// pCmd[8] = ;

		m_pIO->write(pCmd, FEETECH_HEADER + 3);

		return true;
	}

	bool _FeeTech::setSpeed(void)
	{
		IF_F(check() < 0);

		uint16_t s = m_pA->m_s.m_vTarget;

		return true;
	}

	bool _FeeTech::setAccel(void)
	{
		IF_F(check() < 0);

		uint16_t a = m_pA->m_a.m_vTarget;

		return true;
	}

	bool _FeeTech::readStatus(void)
	{
		IF_F(check() < 0);
		IF_T(!m_ieReadStatus.update(m_pT->getTfrom()));

		// uint16_t pB[2];
		// int r = m_pIO->readRegisters(m_ID, m_addr.m_readStat, 2, pB);
		// IF_F(r != 2);
		// m_pT->sleepT(m_cmdInt);

		// //	int p = MAKE32(pB[0], pB[1]);
		// int16_t p = pB[1];
		// m_pA->m_p.m_v = p;

		return true;
	}

	void _FeeTech::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = ((_Console *)pConsole);
		string str = "";

		pC->addMsg(str);
	}

}
