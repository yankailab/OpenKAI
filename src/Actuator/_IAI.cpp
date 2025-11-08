/*
 *  Created on: Nov 8, 2025
 *      Author: yankai
 */
#include "_IAI.h"

namespace kai
{

	_IAI::_IAI()
	{
		m_pMB = nullptr;
		m_iData = 0;

		m_ieCheckAlarm.init(100000);
		m_ieSendCMD.init(50000);
		m_ieReadStatus.init(50000);
	}

	_IAI::~_IAI()
	{
	}

	int _IAI::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iData", &m_iData);
		pK->v("tIntCheckAlarm", &m_ieCheckAlarm.m_tInterval);
		pK->v("tIntSendCMD", &m_ieSendCMD.m_tInterval);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

		return OK_OK;
	}

	int _IAI::link(void)
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

	int _IAI::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _IAI::check(void)
	{
		NULL__(m_pMB, -1);
		IF__(!m_pMB->bOpen(), -1);

		return this->_ActuatorBase::check();
	}

	void _IAI::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			// if (m_bFeedback)
			// {
			checkAlarm();
			//			readStatus();
			//			}

			//		if(!bCmdTimeout())
			//        {
			//			if (m_lastCmdType == actCmd_pos)
//			updatePos();
			//			else if (m_lastCmdType == actCmd_spd)
			//				updateSpeed();
			//        }
		}
	}

	void _IAI::checkAlarm(void)
	{
		IF_(check() != OK_OK);
		IF_(!m_ieCheckAlarm.update(m_pT->getTfrom()));

		for (int i = 0; i < m_vChan.size(); i++)
		{
			ACTUATOR_CHAN *pChan = &m_vChan[i];

			// reset alarm
			// uint8_t pB[10];
			// pB[0] = pChan->getID();
			// pB[1] = 0x45;
			// pB[2] = 0;
			// pB[3] = 0x05;
			// pB[4] = 0;
			// pB[5] = 0;
			// pB[6] = 0;
			// pB[7] = 0;
			// pB[8] = 0x59;
			// pB[9] = 0x03;

			// return to origin
			uint8_t pB[10];
			pB[0] = pChan->getID();
			pB[1] = 0x45;
			pB[2] = 0;
			pB[3] = 0x0A;
			pB[4] = 0;
			pB[5] = 0x07;
			pB[6] = 0;
			pB[7] = 0;
			pB[8] = 0xBC;
			pB[9] = 0xC3;

			int nR = m_pMB->sendRawRequest(pChan->getID(), pB, 8);

			LOG_I(i2str(nR));
		}
	}

	void _IAI::updatePos(void)
	{
		IF_(check() != OK_OK);
		IF_(!m_ieSendCMD.update(m_pT->getTfrom()));

		for (int i = 0; i < m_vChan.size(); i++)
		{
			ACTUATOR_CHAN *pChan = &m_vChan[i];

			int32_t step = pChan->pos()->getTarget();
			int32_t speed = pChan->speed()->getTarget();
			int32_t accel = pChan->accel()->getTarget();
			int32_t brake = pChan->brake()->getTarget();
			int32_t current = pChan->current()->getTarget();

			int32_t inp = 10;

			// create the command
			uint16_t pB[9];
			// 9900h	PCMD target position
			pB[0] = HIGH16(step);
			pB[1] = LOW16(step);
			// 9902		INP	position width
			pB[2] = HIGH16(inp);
			pB[3] = LOW16(inp);
			// 9904		VCMD speed
			pB[4] = HIGH16(speed);
			pB[5] = LOW16(speed);
			// 9906		ACMD accel
			pB[6] = LOW16(accel);
			// 9907		PPOW power limit
			pB[7] = LOW16(current);
			// 9908		CTLF control flag
			pB[8] = 0;

			int nB = m_pMB->writeRegisters(pChan->getID(), // m_iSlave,
									  0x9900,		  // 9900h
									  7,
									  pB);
			if (nB != 7)
			{
				m_ieSendCMD.reset();
			}
		}
	}

	void _IAI::updateSpeed(void)
	{
		IF_(check() != OK_OK);
		IF_(!m_ieSendCMD.update(m_pT->getTfrom()));

		for (int i = 0; i < m_vChan.size(); i++)
		{
			ACTUATOR_CHAN *pChan = &m_vChan[i];

			int32_t step = 0;
			uint8_t dMode = 1;
			int32_t speed = pChan->speed()->getTarget();

			vFloat2 vRange = pChan->pos()->getRange();
			if (speed > 0)
				step = vRange.y;
			else if (speed < 0)
				step = vRange.x;
			else
				dMode = 3;

			int32_t accel = pChan->accel()->getTarget();
			int32_t brake = pChan->brake()->getTarget();
			int32_t current = pChan->current()->getTarget();

			// create the command
			uint16_t pB[18];
			// 88
			pB[0] = 0;
			pB[1] = m_iData;
			pB[2] = 0;
			pB[3] = dMode;
			// 92
			pB[4] = HIGH16(step);
			pB[5] = LOW16(step);
			pB[6] = HIGH16(speed);
			pB[7] = LOW16(speed);

			// 96
			pB[8] = HIGH16(accel);
			pB[9] = LOW16(accel);
			pB[10] = HIGH16(brake);
			pB[11] = LOW16(brake);
			pB[12] = HIGH16(current);
			pB[13] = LOW16(current);
			pB[14] = 0;
			pB[15] = 1;
			pB[16] = 0;
			pB[17] = 0;

			if (m_pMB->writeRegisters(pChan->getID(), 88, 18, pB) != 18)
				m_ieSendCMD.reset();
		}
	}

	void _IAI::readStatus(void)
	{
		IF_(check() != OK_OK);
		IF_(!m_ieReadStatus.update(m_pT->getTfrom()));

		for (int i = 0; i < m_vChan.size(); i++)
		{
			ACTUATOR_CHAN *pChan = &m_vChan[i];

			uint16_t pB[18];
			int nR = 6;
			int r = m_pMB->readRegisters(pChan->getID(), 204, nR, pB);
			IF_(r != 6);

			int32_t p = MAKE32(pB[0], pB[1]);
			int32_t s = MAKE32(pB[4], pB[5]);

			pChan->pos()->set(p);
			pChan->speed()->set(s);

			LOG_I("step: " + f2str(pChan->pos()->get()) + ", speed: " + f2str(pChan->speed()->get()));
		}
	}

}
