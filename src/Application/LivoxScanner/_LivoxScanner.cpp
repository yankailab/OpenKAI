/*
 * _LivoxScanner.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_LivoxScanner.h"

namespace kai
{
	_LivoxScanner::_LivoxScanner()
	{
		m_pLivox = NULL;
		m_msg = "";
	}

	_LivoxScanner::~_LivoxScanner()
	{
	}

	bool _LivoxScanner::init(void *pKiss)
	{
		IF_F(!this->_JSONbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

//		pK->v("iAct", &m_iAct);

		int v;
		v = SEC_2_USEC;
		pK->v("ieSendHB", &v);
		m_ieSendHB.init(v);

		reset();

		return true;
	}

	bool _LivoxScanner::link(void)
	{
		IF_F(!this->_JSONbase::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		F_ERROR_F(pK->v("_LivoxAutoScann", &n));
		m_pLivox = (_LivoxAutoScan *)(pK->getInst(n));
		NULL_Fl(m_pLivox, n + ": not found");

		return true;
	}

	bool _LivoxScanner::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);

		IF_F(!m_pT->start(getUpdate, this));
		IF_F(!m_pTr->start(getUpdateR, this));

		return true;
	}

	int _LivoxScanner::check(void)
	{
		NULL__(m_pLivox, -1);

		return this->_JSONbase::check();
	}

	void _LivoxScanner::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();


			m_pT->autoFPSto();
		}
	}

	void _LivoxScanner::reset(void)
	{
		IF_(check() < 0);

		stop();
		m_pLivox->resetScan();
	}

	void _LivoxScanner::startScan(void)
	{
		IF_(check() < 0);
		m_pLivox->startScan();
	}

	void _LivoxScanner::stop(void)
	{
		IF_(check() < 0);
	}

	void _LivoxScanner::save(void)
	{
		IF_(check() < 0);

		m_pLivox->save();
	}

	void _LivoxScanner::updateR(void)
	{
		while (m_pTr->bRun())
		{
			m_pTr->autoFPSfrom();

			if (recv())
			{
				handleMsg(m_strB);
				m_strB.clear();
			}

			m_pTr->autoFPSto();
		}
	}

	void _LivoxScanner::handleMsg(string &str)
	{
		value json;
		IF_(!str2JSON(str, &json));

		object &jo = json.get<object>();
		IF_(!jo["cmd"].is<string>());
		string cmd = jo["cmd"].get<string>();

		if (cmd == "reset")
			reset(jo);
		else if (cmd == "start")
			start(jo);
		else if (cmd == "stop")
			stop(jo);
		else if (cmd == "save")
			save(jo);
	}

	void _LivoxScanner::reset(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());

		int vID = o["id"].get<double>();
	}

	void _LivoxScanner::start(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		int vID = o["id"].get<double>();

	}

	void _LivoxScanner::stop(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		int vID = o["id"].get<double>();

	}

	void _LivoxScanner::save(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		int vID = o["id"].get<double>();
	}

	void _LivoxScanner::send(void)
	{
		IF_(check() < 0);

		uint64_t t = getApproxTbootUs();

		if (m_ieSendHB.update(t))
			sendHeartbeat();
	}

	void _LivoxScanner::sendHeartbeat(void)
	{
		IF_(check() < 0);

		object o;
		JO(o, "cmd", "hb");
        JO(o, "s", "");
        JO(o, "msg", m_msg);

		sendMsg(o);
	}

	void _LivoxScanner::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_JSONbase::console(pConsole);

		if (m_pTr)
			m_pTr->console(pConsole);
	}

}
