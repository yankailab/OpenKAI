#include "_DDSM.h"

namespace kai
{
	_DDSM::_DDSM()
	{
		m_pTr = NULL;
		m_pIO = NULL;
		m_iMode = 0x02; // default using speed control
	}

	_DDSM::~_DDSM()
	{
		DEL(m_pTr);
	}

	bool _DDSM::init(void *pKiss)
	{
		IF_F(!this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
		

		pK->v("iMode", &m_iMode);

		Kiss *pKt = pK->child("threadR");
		IF_d_F(pKt->empty(), LOG_E("threadR not found"));
		m_pTr = new _Thread();
		if (!m_pTr->init(pKt))
		{
			DEL(m_pTr);
			return false;
		}

		return true;
	}

	bool _DDSM::link(void)
	{
		IF_F(!this->_ActuatorBase::link());
		IF_F(!m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;
		n = "";
		F_ERROR_F(pK->v("_IObase", &n));
		m_pIO = (_IObase *)(pK->findModule(n));
		NULL_Fl(m_pIO, n + ": not found");

		return true;
	}

	bool _DDSM::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->start(getUpdate, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _DDSM::check(void)
	{
		NULL__(m_pIO, -1);
		IF__(!m_pIO->bOpen(), -1);

		return this->_ActuatorBase::check();
	}

	void _DDSM::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			setMode();
			sendCMD();

			m_pT->autoFPSto();
		}
	}

	void _DDSM::sendCMD(void)
	{
		IF_(check() < 0);

		uint8_t pB[DDSM_CMD_NB];
		int nC = m_vAxis.size();
		for (int i = 0; i < nC; i++)
		{
			ACTUATOR_AXIS *pA = &m_vAxis[i];

			pB[0] = i + 1;
			pB[1] = 0x64;
			int16_t s = pA->m_s.m_vTarget;
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
		}
	}

	bool _DDSM::setMode(void)
	{
		IF_F(check() < 0);

		uint8_t pB[DDSM_CMD_NB];
		pB[1] = 0xA0;
		pB[2] = 0;
		pB[3] = 0;
		pB[4] = 0;
		pB[5] = 0;
		pB[6] = 0;
		pB[7] = 0;
		pB[8] = 0;
		pB[9] = m_iMode;

		int nC = m_vAxis.size();
		for (int i = 0; i < nC; i++)
		{
			pB[0] = i + 1;
			m_pIO->write(pB, DDSM_CMD_NB);
		}

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
		IF_F(check() < 0);

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
