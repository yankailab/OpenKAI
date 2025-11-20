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
	}

	_IAI::~_IAI()
	{
	}

	int _IAI::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

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
			uint64_t t = m_pT->getTfrom();

			// bool bR = gotoOrigin();
			// setPos();

			// if (bR)
			// {
			// 	setPos();
			// }
			// else
			// {
			// 	LOG_I(i2str(bR));
			// }

			// if (m_ieReadStatus.update(t))
			// {
			// 	readStatus();
			// }

			if (m_bfSet.b(actuator_clearAlarm, true))
			{
				clearAlarm();
			}

			if (m_bfSet.b(actuator_gotoOrigin, true))
			{
				gotoOrigin();
			}

			if (m_ieSendCMD.update(t))
			{
				if (m_bfSet.b(actuator_move))
				{
					if (!setPos())
						m_ieSendCMD.reset();
				}
				else
				{
					if (!stopMove())
						m_ieSendCMD.reset();
				}
			}
		}
	}

	bool _IAI::clearAlarm(void)
	{
		IF_F(check() != OK_OK);

		uint8_t pB[10];
		pB[0] = m_ID;
		pB[1] = 0x45;
		pB[2] = 0;
		pB[3] = 0x05;
		pB[4] = 0;
		pB[5] = 0;
		pB[6] = 0;
		pB[7] = 0;
		// pB[8] = 0x59;
		// pB[9] = 0x03;

		int nR = m_pMB->sendRawRequest(m_ID, pB, 8);
		LOG_I("clearAlarm: " + i2str(nR));

		return (nR == 8);
	}

	bool _IAI::readStatus(void)
	{
		IF_F(check() != OK_OK);

		// uint16_t pB[18];
		// int nR = 6;
		// int r = m_pMB->readRegisters(m_ID, 204, nR, pB);
		// IF_(r != 6);

		// int32_t p = MAKE32(pB[0], pB[1]);
		// int32_t s = MAKE32(pB[4], pB[5]);

		// m_p.set(p);
		// m_s.set(s);

		// LOG_I("step: " + f2str(m_p.get()) + ", speed: " + f2str(m_s.get()));
	}

	bool _IAI::gotoOrigin(void)
	{
		IF_F(check() != OK_OK);

		uint8_t pB[10];
		pB[0] = m_ID;
		pB[1] = 0x45;
		pB[2] = 0;
		pB[3] = 0x0A;
		pB[4] = 0;
		pB[5] = 0x07;
		pB[6] = 0;
		pB[7] = 0;
		// pB[8] = 0xBC;
		// pB[9] = 0xC3;

		int nR = m_pMB->sendRawRequest(m_ID, pB, 8);
		LOG_I("gotoOrigin: " + i2str(nR));

		return (nR == 8);
	}

	bool _IAI::setPos(void)
	{
		IF_F(check() != OK_OK);

		int32_t step = m_p.getTarget();
		int32_t pErr = m_p.getErrRange();
		int32_t speed = m_s.getTarget();
		int32_t accel = m_a.getTarget();
		int32_t brake = m_b.getTarget();
		int32_t current = m_c.getTarget();

		uint16_t pB[9];
		pB[0] = HIGH16(step); // 9900 PCMD target position
		pB[1] = LOW16(step);
		pB[2] = HIGH16(pErr); // 9902	INP	position width
		pB[3] = LOW16(pErr);
		pB[4] = HIGH16(speed); // 9904	VCMD speed
		pB[5] = LOW16(speed);
		pB[6] = LOW16(accel);	// 9906	ACMD accel
		pB[7] = LOW16(current); // 9907	PPOW power limit
		pB[8] = 0;				// 9908	CTLF control flag

		int nR = m_pMB->writeRegisters(m_ID, 0x9900, 7, pB);
		LOG_I("setPos: " + i2str(nR));

		return (nR == 7);
	}

	bool _IAI::stopMove(void)
	{
		IF_F(check() != OK_OK);

		// uint16_t pB[9];

		// return (m_pMB->writeRegisters(m_ID, 0x9900, 7, pB) == 7);

		return true;
	}

}
