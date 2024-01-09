/*
 *  Created on: Jan 6, 2024
 *      Author: yankai
 */
#include "_Feetech.h"

namespace kai
{

	_Feetech::_Feetech()
	{
		m_port = "";
		m_baud = 115200;
		m_bOpen = false;
		m_ID = 1;
		m_pA = NULL;

		m_ieReadStatus.init(50000);
	}

	_Feetech::~_Feetech()
	{
	}

	bool _Feetech::init(void *pKiss)
	{
		IF_F(!this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("port", &m_port);
		pK->v("baud", &m_baud);
		pK->v("iID", &m_ID);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

		m_pA = &m_vAxis[0];

		return true;
	}

	bool _Feetech::link(void)
	{
		IF_F(!this->_ActuatorBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		return true;
	}

	bool _Feetech::open(void)
	{
		IF_T(m_bOpen);
		IF_F(!m_servo.begin(m_baud, m_port.c_str()));

		m_bOpen = true;
		return true;
	}

	void _Feetech::close(void)
	{
		IF_(!m_bOpen);
		m_servo.end();
		m_bOpen = false;
	}

	bool _Feetech::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _Feetech::check(void)
	{
		IF__(!m_bOpen, -1);
		NULL__(m_pA, -1);

		return this->_ActuatorBase::check();
	}

	void _Feetech::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

            if (!m_bOpen)
            {
                if (!open())
                {
                    LOG_E("Cannot open");
                    m_pT->sleepT(SEC_2_USEC);
                    continue;
                }
            }


			// if (m_bf.b(ACT_BF_STOP, true))
			// {
			// 	while (!readStatus())
			// 		;
			// 	m_pA->m_p.m_vTarget = m_pA->m_p.m_v;
			// }


			updateMove();

			m_pT->autoFPSto();
		}
	}

	void _Feetech::updateMove(void)
	{
		IF_(check() < 0);

		// IF_(!readStatus());
		IF_(!setPos());
		// IF_(!setSpeed());
		// IF_(!setAccel());

	}

	bool _Feetech::setID(uint16_t iSlave)
	{
		IF_F(check() < 0);


		return true;
	}

	bool _Feetech::setBaudrate(uint32_t baudrate)
	{
		IF_F(check() < 0);


		return true;
	}

	bool _Feetech::saveData(void)
	{
		IF_F(check() < 0);

		return true;
	}

	bool _Feetech::setPos(void)
	{
		IF_F(check() < 0);

		m_servo.EnableTorque(1, 1);

		m_servo.WritePosEx(1, 4095, 2400, 50); // 舵机(ID1)以最高速度V=2400(步/秒)，加速度A=50(50*100步/秒^2)，运行至P1=4095位置
		std::cout << "pos = " << 4095 << std::endl;
		usleep(2187 * 1000); //[(P1-P0)/V]*1000+[V/(A*100)]*1000

		m_servo.WritePosEx(1, 0, 2400, 50); // 舵机(ID1)以最高速度V=2400(步/秒)，加速度A=50(50*100步/秒^2)，运行至P0=0位置
		std::cout << "pos = " << 0 << std::endl;
		usleep(2187 * 1000); //[(P1-P0)/V]*1000+[V/(A*100)]*1000

		return true;
	}

	bool _Feetech::setSpeed(void)
	{
		IF_F(check() < 0);

		uint16_t s = m_pA->m_s.m_vTarget;

		return true;
	}

	bool _Feetech::setAccel(void)
	{
		IF_F(check() < 0);

		uint16_t a = m_pA->m_a.m_vTarget;

		return true;
	}

	bool _Feetech::readStatus(void)
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

	void _Feetech::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = ((_Console *)pConsole);
		string str = "";

		pC->addMsg(str);
	}

}
