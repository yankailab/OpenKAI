/*
 * _RopewayScan.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_RopewayScan.h"

namespace kai
{
	_RopewayScan::_RopewayScan()
	{
//		m_pNav = NULL;
		m_pTw = NULL;
		m_pAct = NULL;
		m_iAct = 0;
//		m_pLivox = NULL;
//		m_pVz = NULL;
		m_msg = "";

		m_fProcess.clearAll();
	}

	_RopewayScan::~_RopewayScan()
	{
		DEL(m_pTw);
	}

	bool _RopewayScan::init(void *pKiss)
	{
		IF_F(!this->_JSONbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iAct", &m_iAct);
		pK->v("dist", &m_dist);
		pK->v("spd", &m_spd);
		pK->v("direction", &m_direction);

		int v;
		v = SEC_2_USEC;
		pK->v("ieSendHB", &v);
		m_ieSendHB.init(v);

		Kiss *pKt = pK->child("threadW");
		IF_F(pKt->empty());
		m_pTw = new _Thread();
		if (!m_pTw->init(pKt))
		{
			DEL(m_pTw);
			return false;
		}

		reset();

		return true;
	}

	bool _RopewayScan::link(void)
	{
		IF_F(!this->_JSONbase::link());
		IF_F(!m_pTw->link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n = "";
		// F_ERROR_F(pK->v("_NavBase", &n));
		// m_pNav = (_NavBase *)(pK->findModule(n));
		// NULL_Fl(m_pNav, n + ": not found");

		n = "";
		F_ERROR_F(pK->v("_ActuatorBase", &n));
		m_pAct = (_ActuatorBase *)(pK->findModule(n));
		NULL_Fl(m_pAct, n + ": not found");

		// n = "";
		// F_ERROR_F(pK->v("_RopewayScanLivox", &n));
		// m_pLivox = (_RopewayScanLivox *)(pK->findModule(n));
		// NULL_Fl(m_pLivox, n + ": not found");

		// n = "";
		// F_ERROR_F(pK->v("_RopewayScanVz", &n));
		// m_pVz = (_RopewayScanVz *)(pK->findModule(n));
		// NULL_Fl(m_pVz, n + ": not found");

		return true;
	}

	bool _RopewayScan::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		NULL_F(m_pTw);

		IF_F(!m_pT->start(getUpdate, this));
		IF_F(!m_pTr->start(getUpdateR, this));
		IF_F(!m_pTw->start(getUpdateW, this));

		return true;
	}

	int _RopewayScan::check(void)
	{
//		NULL__(m_pNav, -1);
		NULL__(m_pAct, -1);
//		NULL__(m_pLivox, -1);
//		NULL__(m_pVz, -1);

		return this->_JSONbase::check();
	}

	void _RopewayScan::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			scanUpdate();

			if (m_fProcess.b(rws_reset, true))
				reset();

			if (m_fProcess.b(rws_move, true))
				move();

			if (m_fProcess.b(rws_stop, true))
				hold();

			if (m_fProcess.b(rws_take, true))
				take();

			if (m_fProcess.b(rws_save, true))
				save();

			m_pT->autoFPSto();
		}
	}

	void _RopewayScan::scanUpdate(void)
	{
		IF_(check() < 0);

		// vFloat3 vT = m_pNav->t();

		// m_msg = "POS = (" + f2str(vT.x) + ", " + f2str(vT.y) + ", " +f2str(vT.z) + ")</br>";
		// m_msg += "MEM Vzense =" + f2str(m_pVz->getBufferCap()) + "</br>";
		// m_msg += "MEM Livox =" + f2str(m_pLivox->getBufferCap()) + "</br>";
	}

	void _RopewayScan::reset(void)
	{
		IF_(check() < 0);

		hold();
		// m_pVz->reset();
		// m_pLivox->resetScan();
	}

	void _RopewayScan::move(void)
	{
		IF_(check() < 0);

		m_v = m_spd * m_direction;
		m_pAct->setStarget(m_iAct, m_v);
	}

	void _RopewayScan::stop(void)
	{
		IF_(check() < 0);
		m_pAct->setStarget(m_iAct, 0);
	}

	void _RopewayScan::take(void)
	{
		IF_(check() < 0);

		// m_pVz->take();
		// m_pLivox->takeScan();
	}

	void _RopewayScan::save(void)
	{
		IF_(check() < 0);

		// m_pVz->save();
		// m_pLivox->save();
	}

	void _RopewayScan::updateR(void)
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

	void _RopewayScan::handleMsg(string &str)
	{
		value json;
		IF_(!str2JSON(str, &json));

		object &jo = json.get<object>();
		IF_(!jo["cmd"].is<string>());
		string cmd = jo["cmd"].get<string>();

		if (cmd == "reset")
			reset(jo);
		else if (cmd == "move")
			move(jo);
		else if (cmd == "stop")
			stop(jo);
		else if (cmd == "take")
			take(jo);
		else if (cmd == "save")
			save(jo);
	}

	void _RopewayScan::reset(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());

		int vID = o["id"].get<double>();

		m_fProcess.set(rws_reset);
	}

	void _RopewayScan::move(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		IF_(!o["spd"].is<double>());
		IF_(!o["direction"].is<double>());
		IF_(!o["dist"].is<double>());

		int vID = o["id"].get<double>();
		m_spd = o["spd"].get<double>();
		m_direction = o["direction"].get<double>();
		m_dist = o["dist"].get<double>();

		m_fProcess.set(rws_move);
	}

	void _RopewayScan::stop(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		int vID = o["id"].get<double>();

		m_fProcess.set(rws_stop);
	}

	void _RopewayScan::take(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		int vID = o["id"].get<double>();

		m_fProcess.set(rws_take);
	}

	void _RopewayScan::save(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		int vID = o["id"].get<double>();

		m_fProcess.set(rws_save);
	}

	void _RopewayScan::updateW(void)
	{
		while (m_pTw->bAlive())
		{
			if (!m_pIO)
			{
				m_pTw->sleepT(SEC_2_USEC);
				continue;
			}

			m_pTw->autoFPSfrom();

			send();

			m_pTw->autoFPSto();
		}
	}

	void _RopewayScan::send(void)
	{
		IF_(check() < 0);

		uint64_t t = getApproxTbootUs();

		if (m_ieSendHB.update(t))
			sendHeartbeat();
	}

	void _RopewayScan::sendHeartbeat(void)
	{
		IF_(check() < 0);

		object o;
		JO(o, "cmd", "hb");
        JO(o, "s", "");
        JO(o, "msg", m_msg);

		sendMsg(o);
	}

	void _RopewayScan::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_JSONbase::console(pConsole);

		if (m_pTr)
			m_pTr->console(pConsole);
		if (m_pTw)
			m_pTw->console(pConsole);
	}

}
