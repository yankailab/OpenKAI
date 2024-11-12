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

	int _SBus::init(void *pKiss)
	{
		CHECK_(this->_ProtocolBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bSender", &m_bSender);
		pK->v("timeOutUsec", &m_frame.m_timeOutUsec);
		pK->v("bRawSbus", &m_bRawSbus);
		m_frame.m_nBframe = (m_bRawSbus) ? 25 : SBUS_N_BUF;

		vInt3 vRawRC;
		pK->v("vRawRC", &vRawRC);
		for (int i = 0; i < SBUS_NCHAN; i++)
		{
			RC_CHANNEL *pC = &m_frame.m_pRC[i];
			pC->m_iChan = i;
			pC->m_rawL = vRawRC.x;
			pC->m_rawM = vRawRC.y;
			pC->m_rawH = vRawRC.z;
			pC->update();
		}

		return OK_OK;
	}

	int _SBus::link(void)
	{
		CHECK_(this->_ProtocolBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		return OK_OK;
	}

	int _SBus::start(void)
	{
		if (m_bSender)
		{
			NULL__(m_pT, OK_ERR_NULLPTR);
			CHECK_(m_pT->start(getUpdateW, this));
		}
		else
		{
			NULL__(m_pTr, OK_ERR_NULLPTR);
			CHECK_(m_pTr->start(getUpdateR, this));
		}

		return OK_OK;
	}

	int _SBus::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		return this->_ProtocolBase::check();
	}

	void _SBus::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			send();

			m_pT->autoFPSto();
		}
	}

	void _SBus::send(void)
	{
		IF_(check() != OK_OK);

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
		IF_F(check() != OK_OK);
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
