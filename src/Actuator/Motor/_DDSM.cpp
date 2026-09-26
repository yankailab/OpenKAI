#include "_DDSM.h"

namespace kai
{
	_DDSM::_DDSM()
	{
		m_mode = ddsm_speed;
	}

	_DDSM::~_DDSM()
	{
		DEL(m_pTr);
	}

	bool _DDSM::loadConfig(void)
	{
		IF_F(!this->_ActuatorBase::loadConfig());

		DEL(m_pTr);
		m_pTr = createThread(jK(*m_pJ, "threadR"), "threadR");
		NULL_F(m_pTr);

		return true;
	}

	bool _DDSM::saveConfig(bool bExport)
	{
		if (!_ActuatorBase::saveConfig(false))
		{
			return false;
		}

		if (m_pTr && !m_pTr->saveConfig(false))
		{
			return false;
		}

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _DDSM::link(void)
	{
		IF_F(!this->_ActuatorBase::link());
		const json &j = *m_pJ;
		IF_F(!m_pTr->link());

		string n = "";
		jKv(j, "_IObase", n);
		m_pIO = (_IObase *)(m_pM->findModule(n));
		IF_Le_F(!m_pIO, "_IObase not found: " + n);

		return true;
	}

	bool _DDSM::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->startThread(getUpdate, this));
		return m_pTr->startThread(getUpdateR, this);
	}

	bool _DDSM::check(void)
	{
		NULL_F(m_pIO);
		IF_F(!m_pIO->bOpen());

		return this->_ActuatorBase::check();
	}

	void _DDSM::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			if (m_bfSet.b(actuator_setID))
			{
				if (setID())
					m_bfSet.clear(actuator_setID);
			}

			if (m_bfSet.b(actuator_setMode))
			{
				if (setMode())
					m_bfSet.clear(actuator_setMode);
			}

			if (m_mode == ddsm_speed)
				setOutput(m_s.getTarget());
			else if (m_mode == ddsm_current)
				setOutput(m_c.getTarget());
			else if (m_mode == ddsm_pos)
				setOutput(m_p.getTarget());
		}
	}

	bool _DDSM::setID(void)
	{
		IF_F(!check());

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
		IF_F(!check());

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
		pB[9] = m_mode;
		return m_pIO->write(pB, DDSM_CMD_NB);
	}

	bool _DDSM::setOutput(int16_t v)
	{
		IF_F(!check());

		uint8_t pB[DDSM_CMD_NB];
		pB[0] = m_ID;
		pB[1] = 0x64;
		pB[2] = v >> 8;
		pB[3] = v & 0x00FF;
		pB[4] = 0;
		pB[5] = 0;
		pB[6] = 0;
		pB[7] = 0;
		pB[8] = 0;
		pB[9] = crc8_MAXIM(pB, 9);
		return m_pIO->write(pB, DDSM_CMD_NB);
	}

	void _DDSM::updateR(void)
	{
		while (m_pTr->bRun())
		{
			m_pTr->autoFPS();

			readCMD();
		}
	}

	bool _DDSM::readCMD(void)
	{
		IF_F(!check());

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
