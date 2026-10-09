/*
 * _IObase.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "_IObase.h"

namespace kai
{

	_IObase::_IObase()
	{
	}

	_IObase::~_IObase()
	{
		stop();
		DEL(m_pTr);
	}

	bool _IObase::loadConfig(void)
	{
		stop();
		IF_F(!this->_ModuleBase::loadConfig());
		json &j = *m_pJ;

		if (!j.contains("threadR"))
		{
			// Preserve the existing receive cadence until it is configured separately.
			j["threadR"] = {{"FPS", m_pT->getTargetFPS()}};
		}
		
		DEL(m_pTr);
		m_pTr = createThread(jK(j, "threadR"), "threadR");
		return m_pTr != nullptr;
	}

	bool _IObase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		IF_F(m_pTr && !m_pTr->saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _IObase::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n = "";

		jKv(j, "BytePacketStreamIn", n);
		m_pBpStreamIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pBpStreamIn, "BytePacketStreamIn not found: " + n);
		m_tLastBpStreamIn = 0;

		n.clear();
		jKv(j, "BytePacketStreamOut", n);
		m_pBpStreamOut = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pBpStreamOut, "BytePacketStreamOut not found: " + n);

		return true;
	}

	bool _IObase::start(void)
	{
		if (!m_pT || !m_pTr || !bStopped())
		{
			return false;
		}
		if (!m_pT->startThread(getUpdateW, this) ||
			!m_pTr->startThread(getUpdateR, this))
		{
			stop();
			return false;
		}
		return true;
	}

	void _IObase::pause(void)
	{
		if (m_pT)
		{
			m_pT->pause();
		}
		if (m_pTr)
		{
			m_pTr->pause();
		}
	}

	void _IObase::resume(void)
	{
		if (m_pT)
		{
			m_pT->run();
		}
		if (m_pTr)
		{
			m_pTr->run();
		}
	}

	void _IObase::stop(void)
	{
		// Wake both workers before waiting for either to finish.
		if (m_pT)
		{
			m_pT->stop();
		}
		if (m_pTr)
		{
			m_pTr->stop();
		}
		if (m_pT)
		{
			m_pT->join();
		}
		if (m_pTr)
		{
			m_pTr->join();
		}
	}

	bool _IObase::bRun(void)
	{
		return (m_pT && m_pT->bRun()) || (m_pTr && m_pTr->bRun());
	}

	bool _IObase::bRunning(void)
	{
		return (m_pT && m_pT->bRunning()) || (m_pTr && m_pTr->bRunning());
	}

	bool _IObase::bStopped(void)
	{
		return (!m_pT || m_pT->bStopped()) && (!m_pTr || m_pTr->bStopped());
	}

	_Thread *_IObase::getThread(const string &name)
	{
		if (m_pTr && m_pTr->getName() == name)
		{
			return m_pTr;
		}
		return _ModuleBase::getThread(name);
	}

	void _IObase::updateW(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			// One worker owns reconnect attempts, including receive-only transports.
			if (!bOpen() && !open())
			{
				m_pT->sleepT(NSEC_SEC);
				continue;
			}

			if (m_pBpStreamIn)
			{
				writePackets();
			}
		}
	}

	void _IObase::writePackets(void)
	{
	}

	void _IObase::updateR(void)
	{
		while (m_pTr->bRun())
		{
			m_pTr->autoFPS();

			if (bOpen() && m_pBpStreamOut)
			{
				readPackets();
			}
		}
	}

	void _IObase::readPackets(void)
	{
	}

	bool _IObase::open(void)
	{
		return false;
	}

	bool _IObase::bOpen(void)
	{
		return (m_ioStatus == io_opened);
	}

	void _IObase::close(void)
	{
		m_ioStatus = io_closed;
	}

	IO_STATUS _IObase::getIOstatus(void)
	{
		return m_ioStatus.load();
	}

	void _IObase::setIOstatus(IO_STATUS s)
	{
		m_ioStatus = s;
	}

	void _IObase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		if (m_pTr)
		{
			m_pTr->console(pConsole);
		}
	}

}
