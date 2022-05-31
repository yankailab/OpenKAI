#include "_SBus.h"

namespace kai
{
	_SBus::_SBus()
	{
		m_pTr = NULL;
		m_pIO = NULL;
		m_bSend = false;
		m_bRawSbus = true;

		reset();
	}

	_SBus::~_SBus()
	{
	}

	bool _SBus::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bSend", &m_bSend);
		pK->v("bRawSbus", &m_bRawSbus);
		m_nFrame = (m_bRawSbus) ? 25 : SBUS_NBUF;

		pK->v("vRawRC", &m_vRawRC);
		for (int i = 0; i < SBUS_NCHAN; i++)
		{
			RC_CHANNEL *pC = &m_pRC[i];
			pC->m_iChan = i;
			pC->m_rawL = m_vRawRC.x;
			pC->m_rawM = m_vRawRC.y;
			pC->m_rawH = m_vRawRC.z;
			pC->update();
		}

		Kiss *pKt = pK->child("threadR");
		IF_d_T(pKt->empty(), LOG_E("threadR not found"));

		m_pTr = new _Thread();
		if (!m_pTr->init(pKt))
		{
			DEL(m_pTr);
			return false;
		}
		pKt->m_pInst = m_pTr;

		return true;
	}

	bool _SBus::link(void)
	{
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		F_ERROR_F(pK->v("_IOBase", &n));
		m_pIO = (_IOBase *)(pK->getInst(n));
		NULL_Fl(m_pIO, "_IOBase not found");

		return true;
	}

	bool _SBus::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);

		if (m_bSend)
		{
			IF_F(!m_pT->start(getUpdateW, this));
		}

		return m_pTr->start(getUpdateR, this);
	}

	int _SBus::check(void)
	{
		NULL__(m_pIO, -1);
		IF__(!m_pIO->isOpen(), -1);

		return this->_ModuleBase::check();
	}

	void _SBus::updateW(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			send();

			m_pT->autoFPSto();
		}
	}

	void _SBus::send(void)
	{
		IF_(check() < 0);

		//		m_pIO->write(m_pB, 16);
	}

	void _SBus::updateR(void)
	{
		while (m_pTr->bRun())
		{
			m_pTr->autoFPSfrom();

			if (recv())
			{
				if (m_bRawSbus)
					decodeRaw();
				else
					decode();

				reset();
			}

			m_pTr->autoFPSto();
		}
	}

	bool _SBus::recv(void)
	{
		IF_F(check() < 0);

		uint8_t B;
		while (m_pIO->read(&B, 1) > 0)
		{
			if (m_iB == 0)
			{
				IF_CONT(B != SBUS_HEADER_);
			}

			m_pB[m_iB++] = B;
			IF_CONT(m_iB < m_nFrame);

			m_iB = 0;
			if (!m_bRawSbus)
			{
				uint8_t cs = checksum(&m_pB[1], 33);
				IF_CONT(m_pB[m_nFrame - 1] != cs);
			}

			return true;
		}

		return false;
	}

	void _SBus::decodeRaw(void)
	{
		m_pRC[0].set(static_cast<uint16_t>(m_pB[1] | m_pB[2] << 8 & 0x07FF));
		m_pRC[1].set(static_cast<uint16_t>(m_pB[2] >> 3 | m_pB[3] << 5 & 0x07FF));
		m_pRC[2].set(static_cast<uint16_t>(m_pB[3] >> 6 | m_pB[4] << 2 |
										   m_pB[5] << 10 & 0x07FF));
		m_pRC[3].set(static_cast<uint16_t>(m_pB[5] >> 1 | m_pB[6] << 7 & 0x07FF));
		m_pRC[4].set(static_cast<uint16_t>(m_pB[6] >> 4 | m_pB[7] << 4 & 0x07FF));
		m_pRC[5].set(static_cast<uint16_t>(m_pB[7] >> 7 | m_pB[8] << 1 |
										   m_pB[9] << 9 & 0x07FF));
		m_pRC[6].set(static_cast<uint16_t>(m_pB[9] >> 2 | m_pB[10] << 6 & 0x07FF));
		m_pRC[7].set(static_cast<uint16_t>(m_pB[10] >> 5 | m_pB[11] << 3 & 0x07FF));
		m_pRC[8].set(static_cast<uint16_t>(m_pB[12] | m_pB[13] << 8 & 0x07FF));
		m_pRC[9].set(static_cast<uint16_t>(m_pB[13] >> 3 | m_pB[14] << 5 & 0x07FF));
		m_pRC[10].set(static_cast<uint16_t>(m_pB[14] >> 6 | m_pB[15] << 2 |
											m_pB[16] << 10 & 0x07FF));
		m_pRC[11].set(static_cast<uint16_t>(m_pB[16] >> 1 | m_pB[17] << 7 & 0x07FF));
		m_pRC[12].set(static_cast<uint16_t>(m_pB[17] >> 4 | m_pB[18] << 4 & 0x07FF));
		m_pRC[13].set(static_cast<uint16_t>(m_pB[18] >> 7 | m_pB[19] << 1 |
											m_pB[20] << 9 & 0x07FF));
		m_pRC[14].set(static_cast<uint16_t>(m_pB[20] >> 2 | m_pB[21] << 6 & 0x07FF));
		m_pRC[15].set(static_cast<uint16_t>(m_pB[21] >> 5 | m_pB[22] << 3 & 0x07FF));

		m_bLostFrame = m_pB[23] & SBUS_LOST_FRAME_;
		m_bLostFrame = m_pB[23] & SBUS_FAILSAFE_;
	}

	void _SBus::encodeRaw(void)
	{
		m_pB[0] = SBUS_HEADER_;
		m_pB[1] = static_cast<uint8_t>((m_pRC[0].raw() & 0x07FF));
		m_pB[2] = static_cast<uint8_t>((m_pRC[0].raw() & 0x07FF) >> 8 |
									   (m_pRC[1].raw() & 0x07FF) << 3);
		m_pB[3] = static_cast<uint8_t>((m_pRC[1].raw() & 0x07FF) >> 5 |
									   (m_pRC[2].raw() & 0x07FF) << 6);
		m_pB[4] = static_cast<uint8_t>((m_pRC[2].raw() & 0x07FF) >> 2);
		m_pB[5] = static_cast<uint8_t>((m_pRC[2].raw() & 0x07FF) >> 10 |
									   (m_pRC[3].raw() & 0x07FF) << 1);
		m_pB[6] = static_cast<uint8_t>((m_pRC[3].raw() & 0x07FF) >> 7 |
									   (m_pRC[4].raw() & 0x07FF) << 4);
		m_pB[7] = static_cast<uint8_t>((m_pRC[4].raw() & 0x07FF) >> 4 |
									   (m_pRC[5].raw() & 0x07FF) << 7);
		m_pB[8] = static_cast<uint8_t>((m_pRC[5].raw() & 0x07FF) >> 1);
		m_pB[9] = static_cast<uint8_t>((m_pRC[5].raw() & 0x07FF) >> 9 |
									   (m_pRC[6].raw() & 0x07FF) << 2);
		m_pB[10] = static_cast<uint8_t>((m_pRC[6].raw() & 0x07FF) >> 6 |
										(m_pRC[7].raw() & 0x07FF) << 5);
		m_pB[11] = static_cast<uint8_t>((m_pRC[7].raw() & 0x07FF) >> 3);
		m_pB[12] = static_cast<uint8_t>((m_pRC[8].raw() & 0x07FF));
		m_pB[13] = static_cast<uint8_t>((m_pRC[8].raw() & 0x07FF) >> 8 |
										(m_pRC[9].raw() & 0x07FF) << 3);
		m_pB[14] = static_cast<uint8_t>((m_pRC[9].raw() & 0x07FF) >> 5 |
										(m_pRC[10].raw() & 0x07FF) << 6);
		m_pB[15] = static_cast<uint8_t>((m_pRC[10].raw() & 0x07FF) >> 2);
		m_pB[16] = static_cast<uint8_t>((m_pRC[10].raw() & 0x07FF) >> 10 |
										(m_pRC[11].raw() & 0x07FF) << 1);
		m_pB[17] = static_cast<uint8_t>((m_pRC[11].raw() & 0x07FF) >> 7 |
										(m_pRC[12].raw() & 0x07FF) << 4);
		m_pB[18] = static_cast<uint8_t>((m_pRC[12].raw() & 0x07FF) >> 4 |
										(m_pRC[13].raw() & 0x07FF) << 7);
		m_pB[19] = static_cast<uint8_t>((m_pRC[13].raw() & 0x07FF) >> 1);
		m_pB[20] = static_cast<uint8_t>((m_pRC[13].raw() & 0x07FF) >> 9 |
										(m_pRC[14].raw() & 0x07FF) << 2);
		m_pB[21] = static_cast<uint8_t>((m_pRC[14].raw() & 0x07FF) >> 6 |
										(m_pRC[15].raw() & 0x07FF) << 5);
		m_pB[22] = static_cast<uint8_t>((m_pRC[15].raw() & 0x07FF) >> 3);
		m_pB[23] = 0x00 | (m_bCh17 * SBUS_CH17_) | (m_bCh18 * SBUS_CH18_) |
				   (m_bFailSafe * SBUS_FAILSAFE_) | (m_bLostFrame * SBUS_LOST_FRAME_);
		m_pB[24] = SBUS_FOOTER_;
	}

	void _SBus::decode(void)
	{
		for (int i = 0; i < 16; i++)
		{
			int j = i * 2;
			m_pRC[i].set(static_cast<uint16_t>(m_pB[j + 1] << 8 | m_pB[j + 2]));
		}

		m_flag = m_pB[SBUS_NBUF - 2];
	}

	void _SBus::encode(void)
	{
		for (int i = 0; i < 16; i++)
		{
			int j = i * 2;
			m_pB[j + 1] = static_cast<uint8_t>(m_pRC[i].raw() >> 8);
			m_pB[j + 2] = static_cast<uint8_t>(m_pRC[i].raw() & 0x00FF);
		}

		m_pB[34] = checksum(&m_pB[1], 33);
	}

	uint8_t _SBus::checksum(uint8_t *pB, uint8_t n)
	{
		uint8_t checksum = 0;

		for (int i = 0; i < n; ++i)
			checksum ^= pB[i];

		return checksum;
	}

	void _SBus::reset(void)
	{
		m_iB = 0;
		m_flag = 0;
		m_bLostFrame = false;
		m_bFailSafe = false;
		m_bCh17 = false;
		m_bCh18 = false;
	}

	uint16_t _SBus::raw(int i)
	{
		IF__(i >= SBUS_NCHAN, 0);

		return m_pRC[i].raw();
	}

	float _SBus::v(int i)
	{
		IF__(i >= SBUS_NCHAN, 0);

		return m_pRC[i].v();
	}

	void _SBus::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!m_pIO->isOpen())
			pC->addMsg("Not connected");
		else
			pC->addMsg("Connected");

		pC->addMsg("Raw: " + i2str(m_pRC[0].raw()) + "|" + i2str(m_pRC[1].raw()) + "|" + i2str(m_pRC[2].raw()) + "|" + i2str(m_pRC[3].raw()) + "|" + i2str(m_pRC[4].raw()) + "|" + i2str(m_pRC[5].raw()) + "|" + i2str(m_pRC[6].raw()) + "|" + i2str(m_pRC[7].raw()) + "|" + i2str(m_pRC[8].raw()) + "|" + i2str(m_pRC[9].raw()) + "|" + i2str(m_pRC[10].raw()) + "|" + i2str(m_pRC[11].raw()) + "|" + i2str(m_pRC[12].raw()) + "|" + i2str(m_pRC[13].raw()) + "|" + i2str(m_pRC[14].raw()) + "|" + i2str(m_pRC[15].raw()));

		pC->addMsg("v: " + f2str(m_pRC[0].v(), 2) + "|" + f2str(m_pRC[1].v(), 2) + "|" + f2str(m_pRC[2].v(), 2) + "|" + f2str(m_pRC[3].v(), 2) + "|" + f2str(m_pRC[4].v(), 2) + "|" + f2str(m_pRC[5].v(), 2) + "|" + f2str(m_pRC[6].v(), 2) + "|" + f2str(m_pRC[7].v(), 2) + "|" + f2str(m_pRC[8].v(), 2) + "|" + f2str(m_pRC[9].v(), 2) + "|" + f2str(m_pRC[10].v(), 2) + "|" + f2str(m_pRC[11].v(), 2) + "|" + f2str(m_pRC[12].v(), 2) + "|" + f2str(m_pRC[13].v(), 2) + "|" + f2str(m_pRC[14].v(), 2) + "|" + f2str(m_pRC[15].v(), 2));
	}

}
