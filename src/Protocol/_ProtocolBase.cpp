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
		IF_F(!_ModuleBase::saveConfig(false));


		IF_F(m_pTr && !m_pTr->saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _ProtocolBase::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n;
		jKv(j, "BytePacketStreamIn", n);
		m_pBpStreamIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pBpStreamIn, "BytePacketStreamIn not found: " + n);

		n.clear();
		jKv(j, "BytePacketStreamOut", n);
		m_pBpStreamOut = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pBpStreamOut, "BytePacketStreamOut not found: " + n);
		IF_F(!m_pBpStreamIn && !m_pBpStreamOut);

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
		IF_F(!m_pBpStreamIn && !m_pBpStreamOut);

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
		rCMD.clear();

		while (m_pTr->bRun())
		{
			if (!readCMD(&rCMD))
			{
				m_pTr->autoFPS();
				continue;
			}

			handleCMD(rCMD);
			rCMD.clear();
			m_nCMDrecv++;
		}
	}

	bool _ProtocolBase::readByte(uint8_t *pB)
	{
		NULL_F(m_pBpStreamIn);
		NULL_F(pB);

		if (m_iPacketIn == m_vPacketIn.size())
		{
			m_pBpStreamIn->get(m_vPacketIn, m_tLastBpStreamIn);
			m_iPacketIn = 0;
			m_iByteIn = 0;
		}

		while (m_iPacketIn < m_vPacketIn.size())
		{
			const BYTE_PACKET &packet = m_vPacketIn[m_iPacketIn];
			m_tLastBpStreamIn = packet.m_tStamp;
			if (m_iByteIn < packet.m_vB.size())
			{
				*pB = packet.m_vB[m_iByteIn++];
				return true;
			}

			m_iPacketIn++;
			m_iByteIn = 0;
		}

		return false;
	}

	bool _ProtocolBase::readCMD(PROTOCOL_CMD *pCmd)
	{
		IF_F(!check());
		NULL_F(pCmd);

		uint8_t b;
		while (readByte(&b))
		{
			if (pCmd->input(b))
			{
				return true;
			}
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
