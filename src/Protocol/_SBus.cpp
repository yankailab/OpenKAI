#include "_SBus.h"

namespace kai
{
	_SBus::_SBus()
	{
		m_bSender = false;
		m_bRawSbus = true;

		m_frame.clear();
	}

	_SBus::~_SBus()
	{
	}

	bool _SBus::init(const json& j)
	{
		IF_F(!this->_ProtocolBase::init(j));

		m_bSender = j.value("bSender", "");
		m_frame.m_timeOutUsec = j.value("timeOutUsec", m_frame.m_timeOutUsec);
		m_bRawSbus = j.value("bRawSbus", "");
		m_frame.m_nBframe = (m_bRawSbus) ? 25 : SBUS_N_BUF;

		vInt3 vRawRC;
		vRawRC = j.value("vRawRC", "");
		for (int i = 0; i < SBUS_NCHAN; i++)
		{
			RC_CHANNEL *pC = &m_frame.m_pRC[i];
			pC->m_iChan = i;
			pC->m_rawL = vRawRC.x;
			pC->m_rawM = vRawRC.y;
			pC->m_rawH = vRawRC.z;
			pC->update();
		}

		return true;
	}

	bool _SBus::link(const json& j, ModuleMgr* pM)
	{
		IF_F(!this->_ProtocolBase::link(j, pM));

		return true;
	}

	bool _SBus::start(void)
	{
		if (m_bSender)
		{
			NULL_F(m_pT);
			IF_F(!m_pT->start(getUpdateW, this));
		}
		else
		{
			NULL_F(m_pTr);
			IF_F(!m_pTr->start(getUpdateR, this));
		}

		return true;
	}

	bool _SBus::check(void)
	{
		NULL_F(m_pIO);
		IF_F(!m_pIO->bOpen());

		return this->_ProtocolBase::check();
	}

	void _SBus::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			send();

		}
	}

	void _SBus::send(void)
	{
		IF_(!check());

		//		m_pIO->write(m_pB, 16);
	}

	void _SBus::updateR(void)
	{
		while (m_pTr->bAlive())
		{
			IF_CONT(!readSbus(&m_frame));

			if (m_bRawSbus)
				m_frame.decodeRaw();
			else
				m_frame.decode();

			m_frame.m_tLastRecv = getApproxTbootUs();
			m_frame.clear();
		}
	}

	bool _SBus::readSbus(SBUS_FRAME *pF)
	{
		IF_F(!check());
		NULL_F(pF);

		if (m_nRead == 0)
		{
			m_nRead = m_pIO->read(m_pBuf, PB_N_BUF);
			IF_F(m_nRead <= 0);
			m_iRead = 0;
		}

		while (m_iRead < m_nRead)
		{
			bool r = pF->input(m_pBuf[m_iRead++]);
			if (m_iRead == m_nRead)
			{
				m_iRead = 0;
				m_nRead = 0;
			}

			IF_CONT(!r);

			IF__(m_bRawSbus, true);
			IF__(pF->bChecksum(), true);

			pF->clear();
			continue;
		}

		return false;
	}

	uint16_t _SBus::raw(int iChan)
	{
		IF__(iChan >= SBUS_NCHAN, 0);

		return m_frame.m_pRC[iChan].raw();
	}

	float _SBus::v(int iChan)
	{
		IF__(iChan >= SBUS_NCHAN, 0);

		return m_frame.m_pRC[iChan].v();
	}

	void _SBus::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ProtocolBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!m_pIO->bOpen())
			pC->addMsg("Not connected");
		else
			pC->addMsg("Connected");

		pC->addMsg("Raw: " + i2str(m_frame.m_pRC[0].raw())
					+ "|" + i2str(m_frame.m_pRC[1].raw())
					+ "|" + i2str(m_frame.m_pRC[2].raw())
					+ "|" + i2str(m_frame.m_pRC[3].raw())
					+ "|" + i2str(m_frame.m_pRC[4].raw())
					+ "|" + i2str(m_frame.m_pRC[5].raw())
					+ "|" + i2str(m_frame.m_pRC[6].raw())
					+ "|" + i2str(m_frame.m_pRC[7].raw())
					+ "|" + i2str(m_frame.m_pRC[8].raw())
					+ "|" + i2str(m_frame.m_pRC[9].raw())
					+ "|" + i2str(m_frame.m_pRC[10].raw())
					+ "|" + i2str(m_frame.m_pRC[11].raw())
					+ "|" + i2str(m_frame.m_pRC[12].raw())
					+ "|" + i2str(m_frame.m_pRC[13].raw())
					+ "|" + i2str(m_frame.m_pRC[14].raw())
					+ "|" + i2str(m_frame.m_pRC[15].raw()));

		pC->addMsg("v: " + f2str(m_frame.m_pRC[0].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[1].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[2].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[3].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[4].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[5].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[6].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[7].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[8].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[9].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[10].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[11].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[12].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[13].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[14].v(), 2)
					+ "|" + f2str(m_frame.m_pRC[15].v(), 2));

		if (m_frame.bFailSafe())
			pC->addMsg("FailSafe");
	}

}
