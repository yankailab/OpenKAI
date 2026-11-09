/*
 * _JSONcmd.cpp
 *
 *  Created on: May 6, 2026
 *      Author: yankai
 */

#include "_JSONcmd.h"

namespace kai
{
	_JSONcmd::_JSONcmd()
	{
	}

	_JSONcmd::~_JSONcmd()
	{
		// Join before the derived vtable, dispatch targets and JSON state die.
		stop();
	}

	bool _JSONcmd::loadConfig(void)
	{
		IF_F(!this->_JSONbase::loadConfig());

		// jKv(j, "dirSave", m_dirSave);

		return true;
	}

	bool _JSONcmd::saveConfig(bool bExport)
	{
		IF_F(!_JSONbase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _JSONcmd::link(InstanceMgr *pM)
	{
		IF_F(!this->_JSONbase::link(pM));
		const json &j = *m_pJ;

		vector<string> vB;
		jKv(j, "vBASE", vB);
		m_vpB.clear();
		for (string n : vB)
		{
			BASE *pB = (BASE *)(pM->findModule(n));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		return true;
	}

	bool _JSONcmd::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);

		IF_F(!m_pT->startThread(getUpdate, this));
		if (!m_pTr->startThread(getUpdateR, this))
		{
			m_pT->join();
			return false;
		}

		return true;
	}

	void _JSONcmd::stop(void)
	{
		// Stop remains valid after IO closes and before link()/start() succeeds.
		// Signal both workers before joining: the receiver can wake the sender.
		if (m_pT) m_pT->stop();
		if (m_pTr) m_pTr->stop();
		if (m_pTr) m_pTr->join();
		if (m_pT) m_pT->join();
	}

	bool _JSONcmd::check(void)
	{
		return this->_JSONbase::check();
	}

	void _JSONcmd::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun()) break;

			send();
		}
	}

	void _JSONcmd::send(void)
	{
		IF_(!check());

		uint64_t t = getTns();

		if (m_ieSendHB.update(t))
			sendHeartbeat();
	}

	void _JSONcmd::sendHeartbeat(void)
	{
		IF_(!check());

		json j = json::object();
		j["cmd"] = "hb";

		// bool r = sendJson(j);
		sendJson(j);
	}

	void _JSONcmd::sendConfig(void)
	{
		IF_(!check());
	}

	void _JSONcmd::updateR(void)
	{
		string strR = "";

		while (m_pTr->bRun())
		{
			m_pTr->autoFPS();
			if (!m_pTr->bRun()) break;

			IF_CONT(!recvJson(&strR));

			m_pT->skipSleep();

			handleJson(strR);
			strR.clear();
		}
	}

	void _JSONcmd::handleJson(const string &str)
	{
		json j;
		IF_(!str2JSON(str, j));

		string m;
		IF_(!jKv(j, "module", m));

		BASE *pB = findModule(m);
		NULL_(pB);

		pB->jsonCmd(j, (void*)this);
	}

	BASE *_JSONcmd::findModule(const string &n)
	{
		for (BASE *pB : m_vpB)
		{
			IF_CONT(n != pB->getName());

			return pB;
		}

		return nullptr;
	}

	void _JSONcmd::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_JSONbase::console(pConsole);

		if (m_pTr)
			m_pTr->console(pConsole);
	}

}
