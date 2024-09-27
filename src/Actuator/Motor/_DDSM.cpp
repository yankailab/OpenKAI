#include "_DDSM.h"

namespace kai
{
	_DDSM::_DDSM()
	{
		m_pTr = nullptr;
		m_pIO = nullptr;

		m_ID = 1;
		m_iMode = 0x02; // default using speed control
	}

	_DDSM::~_DDSM()
	{
		DEL(m_pTr);
	}

	int _DDSM::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("ID", &m_ID);
		pK->v("iMode", &m_iMode);

		Kiss *pKt = pK->child("threadR");
		if (pKt->empty())
		{
			LOG_E("threadR not found");
			return OK_ERR_NOT_FOUND;
		}

		m_pTr = new _Thread();
		CHECK_d_(m_pTr->init(pKt), DEL(m_pTr));

		return OK_OK;
	}

	int _DDSM::link(void)
	{
		CHECK_(this->_ActuatorBase::link());
		CHECK_(m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;
		
		n = "";
		IF__(!pK->v("_IObase", &n), OK_ERR_NOT_FOUND);
		m_pIO = (_IObase *)(pK->findModule(n));
		NULL__(m_pIO, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _DDSM::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdate, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _DDSM::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		return this->_ActuatorBase::check();
	}

	void _DDSM::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if(m_bfSet.b(actuator_setID))
			{
				if(setID())
					m_bfSet.clear(actuator_setID);
			}
	
			if(m_bfSet.b(actuator_setMode))
			{
				if(setMode())
					m_bfSet.clear(actuator_setMode);
			}
	
			if(m_mode == actuatorMode_speed)
				setSpeed();

			m_pT->autoFPSto();
		}
	}

	bool _DDSM::setID(void)
	{
		IF_F(check() != OK_OK);

		uint8_t pB[DDSM_CMD_NB];
		pB[0] = 0xAA;
		pB[1] = 0x55;
		pB[2] = 0x53;
		pB[3] = m_ID;
		pB[4] = 0;
		pB[5] = 0;
		pB[6] = 0;
		pB[7] = 0;
		pB[8] = 0;
		pB[9] = crc8_MAXIM(pB, 9);

		for (int i = 0; i < 5; i++)
		{
			m_pIO->write(pB, DDSM_CMD_NB);
		}

		return true;
	}

	bool _DDSM::setMode(void)
	{
		IF_F(check() != OK_OK);

		uint8_t pB[DDSM_CMD_NB];
		pB[0] = m_ID;
		pB[1] = 0xA0;
		pB[2] = 0;
		pB[3] = 0;
		pB[4] = 0;
		pB[5] = 0;
		pB[6] = 0;
		pB[7] = 0;
		pB[8] = 0;
		pB[9] = m_iMode;
		m_pIO->write(pB, DDSM_CMD_NB);

		return true;
	}

	bool _DDSM::setSpeed(void)
	{
		IF_F(check() != OK_OK);

		uint8_t pB[DDSM_CMD_NB];
		pB[0] = m_ID;
		pB[1] = 0x64;
		int16_t s = m_s.m_vTarget;
		pB[2] = s >> 8;
		pB[3] = s & 0x00FF;
		pB[4] = 0;
		pB[5] = 0;
		pB[6] = 0;
		pB[7] = 0;
		pB[8] = 0;
		pB[9] = crc8_MAXIM(pB, 9);
		m_pIO->write(pB, DDSM_CMD_NB);

		// pA->m_s.m_v = pA->m_s.m_vTarget;

		return true;
	}

	void _DDSM::updateR(void)
	{
		while (m_pTr->bAlive())
		{
			m_pTr->autoFPSfrom();

			readCMD();

			m_pTr->autoFPSto();
		}
	}

	bool _DDSM::readCMD(void)
	{
		IF_F(check() != OK_OK);

		return false;
	}

	void _DDSM::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ActuatorBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!m_pIO->bOpen())
		{
			pC->addMsg("Not Connected", 1);
			return;
		}
	}

}
