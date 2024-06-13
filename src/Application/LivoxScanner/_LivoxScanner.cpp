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

		return true;
	}

	bool _LivoxScanner::link(void)
	{
		IF_F(!this->_JSONbase::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		F_ERROR_F(pK->v("_LivoxAutoScan", &n));
		m_pLivox = (_LivoxAutoScan *)(pK->findModule(n));
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
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			send();

			m_pT->autoFPSto();
		}
	}

	void _LivoxScanner::updateR(void)
	{
		while (m_pTr->bAlive())
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
		else if (cmd == "setConfig")
			setConfig(jo);
		else if (cmd == "getConfig")
			getConfig(jo);
	}

	void _LivoxScanner::reset(picojson::object &o)
	{
		IF_(check() < 0);
		m_pLivox->reset();

		m_msg = "Memory cleared";
	}

	void _LivoxScanner::start(picojson::object &o)
	{
		IF_(check() < 0);
		m_pLivox->startAuto();

		m_msg = "Scan started";
	}

	void _LivoxScanner::stop(picojson::object &o)
	{
		IF_(check() < 0);
		m_pLivox->stop();

		m_msg = "Scan stopped";
	}

	void _LivoxScanner::save(picojson::object &o)
	{
		IF_(check() < 0);
		m_pLivox->save();

		m_msg = "Point cloud files saved";
	}

	void _LivoxScanner::setConfig(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["vFrom"].is<double>());
		IF_(!o["vStep"].is<double>());
		IF_(!o["vTo"].is<double>());
		IF_(!o["hFrom"].is<double>());
		IF_(!o["hStep"].is<double>());
		IF_(!o["hTo"].is<double>());
		IF_(!o["Xoffset"].is<double>());
		IF_(!o["Yoffset"].is<double>());
		IF_(!o["Zoffset"].is<double>());

		LivoxAutoScanConfig c;
		c.m_vRangeV.x = o["vFrom"].get<double>();
		c.m_vRangeV.y = o["vTo"].get<double>();
		c.m_dV = o["vStep"].get<double>();
		c.m_vRangeH.x = o["hFrom"].get<double>();
		c.m_vRangeH.y = o["hTo"].get<double>();
		c.m_dH = o["hStep"].get<double>();
		c.m_vOffset.x = o["Xoffset"].get<double>();
		c.m_vOffset.y = o["Yoffset"].get<double>();
		c.m_vOffset.z = o["Zoffset"].get<double>();

		m_pLivox->setConfig(c);

		sendConfig();
	}

	void _LivoxScanner::getConfig(picojson::object &o)
	{
		IF_(check() < 0);

		sendConfig();
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

		if (m_msg.empty())
		{
			m_msg = "Standby: ";
			if (m_pLivox->bScanning())
				m_msg = "Scanning: ";

			m_msg += i2str(m_pLivox->getBufferCap() * 100) + "% memory used";
		}

		object o;
		JO(o, "cmd", "hb");
		JO(o, "s", "");
		JO(o, "msg", m_msg);

		m_msg = "";

		sendMsg(o);
	}

	void _LivoxScanner::sendConfig(void)
	{
		IF_(check() < 0);

		m_msg = "Config updated";
		LivoxAutoScanConfig c = m_pLivox->getConfig();

		object o;
		JO(o, "cmd", "getConfig");
		JO(o, "msg", m_msg);
		JO(o, "s", "");
		JO(o, "vFrom", c.m_vRangeV.x);
		JO(o, "vTo", c.m_vRangeV.y);
		JO(o, "vStep", c.m_dV);
		JO(o, "hFrom", c.m_vRangeH.x);
		JO(o, "hTo", c.m_vRangeH.y);
		JO(o, "hStep", c.m_dH);
		JO(o, "Xoffset", c.m_vOffset.x);
		JO(o, "Yoffset", c.m_vOffset.y);
		JO(o, "Zoffset", c.m_vOffset.z);

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
