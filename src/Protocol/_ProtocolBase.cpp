#include "_ProtocolBase.h"

namespace kai
{

	_ProtocolBase::_ProtocolBase()
	{
	}

	_ProtocolBase::~_ProtocolBase()
	{
		DEL(m_pTr);
	}

	bool _ProtocolBase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());

		DEL(m_pTr);
		m_pTr = createThread(jK(*m_pJ, "threadR"), "threadR");
		NULL_F(m_pTr);

		return true;
	}

	bool _ProtocolBase::saveConfig(bool bExport)
	{
		if (!_ModuleBase::saveConfig(false))
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

	bool _ProtocolBase::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;
		IF_F(!m_pTr->link());

		string n = "";
		jKv(j, "_IObase", n);
		m_pIO = (_IObase *)(m_pM->findModule(n));
		NULL_F(m_pIO);

		return true;
	}

	bool _ProtocolBase::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->startThread(getUpdateW, this));
		return m_pTr->startThread(getUpdateR, this);
	}

	bool _ProtocolBase::check(void)
	{
		NULL_F(m_pIO);
		IF_F(!m_pIO->bOpen());

		return this->_ModuleBase::check();
	}

	void _ProtocolBase::updateW(void)
	{
		while (m_pT->bRun())
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

		while (m_pTr->bRun())
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

		if (m_pTr)
			m_pTr->console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("nCMD = " + i2str(m_nCMDrecv), 1);
	}

}
