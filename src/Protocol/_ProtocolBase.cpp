#include "_ProtocolBase.h"

namespace kai
{

	_ProtocolBase::_ProtocolBase()
	{
		m_pTr = nullptr;
		m_pIO = nullptr;
		m_nCMDrecv = 0;

		m_nRead = 0;
		m_iRead = 0;
	}

	_ProtocolBase::~_ProtocolBase()
	{
		DEL(m_pTr);
	}

	int _ProtocolBase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pKt = pK->child("threadR");
		if (pKt->empty())
		{
			LOG_E("threadR not found");
			return OK_ERR_NOT_FOUND;
		}

		m_pTr = new _Thread();
		CHECK_d_l_(m_pTr->init(pKt), DEL(m_pTr), "threadR init failed");

		return OK_OK;
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
//		NULL__(m_pIO, OK_ERR_NOT_FOUND);

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

		return this->_ModuleBase::check();
	}

	void _ProtocolBase::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			send();

		}
	}

	void _ProtocolBase::send(void)
	{
		IF_(check() != OK_OK);
	}

	void _ProtocolBase::updateR(void)
	{
		PROTOCOL_CMD rCMD;

		while (m_pTr->bAlive())
		{
			m_pTr->autoFPS();

			IF_CONT(!readCMD(&rCMD));

			m_pTr->skipSleep();

			handleCMD(rCMD);
			rCMD.clear();
			m_nCMDrecv++;
		}
	}

	bool _ProtocolBase::readCMD(PROTOCOL_CMD *pCmd)
	{
		IF_F(check() != OK_OK);
		NULL_F(pCmd);

		if (m_nRead == 0)
		{
			m_nRead = m_pIO->read(m_pBuf, PB_N_BUF);
			IF_F(m_nRead <= 0);
			m_iRead = 0;
		}

		while (m_iRead < m_nRead)
		{
			bool r = pCmd->input(m_pBuf[m_iRead++]);
			if (m_iRead == m_nRead)
			{
				m_iRead = 0;
				m_nRead = 0;
			}

			IF__(r, true);
		}

		return false;
	}

	void _ProtocolBase::handleCMD(const PROTOCOL_CMD &cmd)
	{
	}

	void _ProtocolBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (m_pIO)
		{
			if (!m_pIO->bOpen())
			{
				pC->addMsg("Not Connected", 1);
				return;
			}
		}

		pC->addMsg("nCMD = " + i2str(m_nCMDrecv), 1);
	}

}
