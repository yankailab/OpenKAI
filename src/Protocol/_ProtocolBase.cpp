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

	int _ProtocolBase::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

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

	int _ProtocolBase::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));
		CHECK_(m_pTr->link());

		string n;

		n = "";
		= j.value("_IObase", &n);
		m_pIO = (_IObase *)(pM->findModule(n));
		NULL_F(m_pIO);

		return true;
	}

	int _ProtocolBase::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _ProtocolBase::check(void)
	{
		NULL_F(m_pIO);
		IF_F(!m_pIO->bOpen());

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
		IF_(!check());
	}

	void _ProtocolBase::updateR(void)
	{
		PROTOCOL_CMD rCMD;

		while (m_pTr->bAlive())
		{
			IF_CONT(!readCMD(&rCMD));

			handleCMD(rCMD);
			rCMD.clear();
			m_nCMDrecv++;
		}
	}

	bool _ProtocolBase::readCMD(PROTOCOL_CMD *pCmd)
	{
		IF_F(!check());
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

		if(m_pTr)
			m_pTr->console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("nCMD = " + i2str(m_nCMDrecv), 1);
	}

}
