#include "_DDSM.h"

namespace kai
{
	_DDSM::_DDSM()
	{
		m_pTr = nullptr;
		m_pIO = nullptr;

		// m_ID = 1;
		// m_iMode = 0x02; // default using speed control
	}

	_DDSM::~_DDSM()
	{
		DEL(m_pTr);
	}

	int _DDSM::init(void *pKiss)
	{
		CHECK_(this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		// pK->v("ID", &m_ID);
		// pK->v("iMode", &m_iMode);

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
			m_pT->autoFPS();

			for (int i=0; i<m_vChan.size(); i++)
			{
				ACTUATOR_CHAN* pChan = &m_vChan[i];

				if (pChan->m_bfSet.b(actuator_setID))
				{
					if (setID(i))
						pChan->m_bfSet.clear(actuator_setID);
				}

				if (pChan->m_bfSet.b(actuator_setMode))
				{
					if (setMode(i))
						pChan->m_bfSet.clear(actuator_setMode);
				}

				if (pChan->m_mode == 0x02)	// speed mode
					setSpeed(i);
			}
		}
	}

	bool _DDSM::setID(int iChan)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan(iChan);
		NULL_F(pChan);

		uint8_t pB[DDSM_CMD_NB];
		pB[0] = 0xAA;
		pB[1] = 0x55;
		pB[2] = 0x53;
		pB[3] = pChan->getID();
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

	bool _DDSM::setMode(int iChan)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan(iChan);
		NULL_F(pChan);

		uint8_t pB[DDSM_CMD_NB];
		pB[0] = pChan->getID();
		pB[1] = 0xA0;
		pB[2] = 0;
		pB[3] = 0;
		pB[4] = 0;
		pB[5] = 0;
		pB[6] = 0;
		pB[7] = 0;
		pB[8] = 0;
		pB[9] =  pChan->getMode();
		m_pIO->write(pB, DDSM_CMD_NB);

		return true;
	}

	bool _DDSM::setSpeed(int iChan)
	{
		IF_F(check() != OK_OK);
		ACTUATOR_CHAN* pChan = getChan(iChan);
		NULL_F(pChan);

		uint8_t pB[DDSM_CMD_NB];
		pB[0] = pChan->getID();
		pB[1] = 0x64;
		int16_t s = pChan->speed()->getTarget();
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
			m_pTr->autoFPS();

			readCMD();
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
