#include "_ProtocolBase.h"

namespace kai
{

	_ProtocolBase::_ProtocolBase()
	{
		m_pTr = nullptr;
		m_pIO = nullptr;
		m_pBuf = nullptr;
		m_nBuf = 256;
		m_nCMDrecv = 0;
	}

	_ProtocolBase::~_ProtocolBase()
	{
	}

	int _ProtocolBase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nBuf", &m_nBuf);
		m_pBuf = new uint8_t[m_nBuf];
		m_recvMsg.init(m_nBuf);

        Kiss *pKt = pK->child("threadR");
        if (pKt->empty())
        {
            LOG_E("threadR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTr = new _Thread();
        CHECK_d_l_(m_pTr->init(pKt), DEL(m_pTr), "threadR init failed");

		return true;
	}

    int _ProtocolBase::link(void)
    {
        CHECK_(this->_ModuleBase::link());
        CHECK_(m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;
		n = "";
		pK->v("_IObase", &n);
		m_pIO = (_IObase *)(pK->findModule(n));
		NULL__(m_pIO, OK_ERR_NOT_FOUND);

        return OK_OK;
    }

	int _ProtocolBase::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _ProtocolBase::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);
		NULL__(m_pBuf, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _ProtocolBase::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			send();

			m_pT->autoFPSto();
		}
	}

	void _ProtocolBase::send(void)
	{
		IF_(check() != OK_OK);

		//m_pIO->write(m_pB, 16);
	}

	void _ProtocolBase::updateR(void)
	{
		while (m_pTr->bAlive())
		{
			if (!m_pIO)
			{
				m_pTr->sleepT(SEC_2_USEC);
				continue;
			}

			if (!m_pIO->bOpen())
			{
				m_pTr->sleepT(SEC_2_USEC);
				continue;
			}

			m_pTr->autoFPSfrom();

			while (readCMD())
			{
				handleCMD();
				m_recvMsg.reset();
				m_nCMDrecv++;
			}

			m_pTr->autoFPSto();
		}
	}

	bool _ProtocolBase::readCMD(void)
	{
		IF_F(check() != OK_OK);

		uint8_t b;
		while (m_pIO->read(&b, 1) > 0)
		{
			if (m_recvMsg.m_cmd != 0)
			{
				m_recvMsg.m_pB[m_recvMsg.m_iB] = b;
				m_recvMsg.m_iB++;

				if (m_recvMsg.m_iB == 3)
				{
					m_recvMsg.m_nPayload = m_recvMsg.m_pB[2];
				}

				IF__(m_recvMsg.m_iB == m_recvMsg.m_nPayload + PB_N_HDR, true);
			}
			else if (b == PB_BEGIN)
			{
				m_recvMsg.m_cmd = b;
				m_recvMsg.m_pB[0] = b;
				m_recvMsg.m_iB = 1;
				m_recvMsg.m_nPayload = 0;
			}
		}

		return false;
	}

	void _ProtocolBase::handleCMD(void)
	{
	}

	void _ProtocolBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!m_pIO->bOpen())
		{
			pC->addMsg("Not Connected", 1);
			return;
		}

		pC->addMsg("nCMD = " + i2str(m_nCMDrecv), 1);
	}

}
