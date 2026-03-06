/*
 * _WSctrl.cpp
 *
 *  Created on: May 6, 2026
 *      Author: yankai
 */

#include "_WSctrl.h"

namespace kai
{
	_WSctrl::_WSctrl()
	{
	}

	_WSctrl::~_WSctrl()
	{
	}

	bool _WSctrl::init(const json &j)
	{
		IF_F(!this->_JSONbase::init(j));

		// jKv(j, "dirSave", m_dirSave);

		return true;
	}

	bool _WSctrl::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_JSONbase::link(j, pM));

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

	bool _WSctrl::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);

		IF_F(!m_pT->start(getUpdate, this));
		IF_F(!m_pTr->start(getUpdateR, this));

		return true;
	}

	bool _WSctrl::check(void)
	{
		return this->_JSONbase::check();
	}

	void _WSctrl::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			send();
		}
	}

	void _WSctrl::send(void)
	{
		IF_(!check());

		uint64_t t = getApproxTbootUs();

		if (m_ieSendHB.update(t))
			sendHeartbeat();
	}

	void _WSctrl::sendHeartbeat(void)
	{
		IF_(!check());

		json j = json::object();
		j["cmd"] = "hb";

		bool r = sendJson(j);
	}

	void _WSctrl::sendConfig(void)
	{
		IF_(!check());
	}

	void _WSctrl::updateR(void)
	{
		string strR = "";

		while (m_pTr->bAlive())
		{
			m_pTr->autoFPS();

			IF_CONT(!recvJson(&strR, m_pIO));

			m_pT->skipSleep();

			handleJson(strR);
			strR.clear();
		}
	}

	void _WSctrl::handleJson(const string &str)
	{
		json j;
		IF_(!str2JSON(str, j));

		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		// if (cmd == "setPosSpd")
		// 	onSetPosSpd(j);
		// else if (cmd == "clearAlarm")
		// 	onClearAlarm(j);
		// else if (cmd == "initOrigin")
		// 	onInitOrigin(j);
	}

	void _WSctrl::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_JSONbase::console(pConsole);

		if (m_pTr)
			m_pTr->console(pConsole);
	}

}
