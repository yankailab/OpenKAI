/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_OrientalMotor.h"

namespace kai
{

	_OrientalMotor::_OrientalMotor()
	{
		m_pMB = nullptr;
		m_iData = 0;
	}

	_OrientalMotor::~_OrientalMotor()
	{
	}

	int _OrientalMotor::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iData", &m_iData);

		return OK_OK;
	}

	int _OrientalMotor::link(void)
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

	int _OrientalMotor::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _OrientalMotor::check(void)
	{
		NULL__(m_pMB, -1);
		IF__(!m_pMB->bOpen(), -1);

		return this->_ActuatorBase::check();
	}

	void _OrientalMotor::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
			uint64_t t = m_pT->getTfrom();

			clearAlarm();

			if (!m_ieReadStatus.update(t))
			{
				readStatus();
			}

			if (m_ieSendCMD.update(t))
			{
				if (!setPos())
					m_ieSendCMD.reset();

				if (!setSpeed())
					m_ieSendCMD.reset();
			}
		}
	}

	bool _OrientalMotor::clearAlarm(void)
	{
		IF_F(check() != OK_OK);

		uint16_t pB[2];
		pB[0] = 1 << 7;
		pB[1] = 0;
		m_pMB->writeRegisters(m_ID, 125, 1, pB);
	}

	bool _OrientalMotor::readStatus(void)
	{
		IF_F(check() != OK_OK);

		uint16_t pB[18];
		int nB = 6;
		int nR = m_pMB->readRegisters(m_ID, 204, nB, pB);
		IF_F(nR != nB);

		int32_t p = MAKE32(pB[0], pB[1]);
		int32_t s = MAKE32(pB[4], pB[5]);

		m_p.set(p);
		m_s.set(s);

		LOG_I("step: " + f2str(m_p.get()) + ", speed: " + f2str(m_s.get()));

		return true;
	}

	bool _OrientalMotor::setPos(void)
	{
		IF_F(check() != OK_OK);

		int32_t step = m_p.getTarget();
		int32_t speed = m_s.getTarget();
		int32_t accel = m_a.getTarget();
		int32_t brake = m_b.getTarget();
		int32_t current = m_c.getTarget();

		// create the command
		uint16_t pB[18];
		// 88
		pB[0] = 0;
		pB[1] = m_iData;
		pB[2] = 0;
		pB[3] = 1;
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

		return (m_pMB->writeRegisters(m_ID, 88, 18, pB) == 18);
	}

	bool _OrientalMotor::setSpeed(void)
	{
		IF_F(check() != OK_OK);

		int32_t step = 0;
		uint8_t dMode = 1;
		int32_t speed = m_s.getTarget();
		vFloat2 vRange = m_p.getRange();
		if (speed > 0)
			step = vRange.y;
		else if (speed < 0)
			step = vRange.x;
		else
			dMode = 3;

		int32_t accel = m_a.getTarget();
		int32_t brake = m_b.getTarget();
		int32_t current = m_c.getTarget();

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

		return (m_pMB->writeRegisters(m_ID, 88, 18, pB) == 18);
	}

}
