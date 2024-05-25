/*
 * _GSVctrl.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_GSVctrl.h"

namespace kai
{
	_GSVctrl::_GSVctrl()
	{
		m_pGgrid = NULL;
		m_msg = "";
	}

	_GSVctrl::~_GSVctrl()
	{
	}

	bool _GSVctrl::init(void *pKiss)
	{
		IF_F(!this->_JSONbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		//		pK->v("iAct", &m_iAct);

		return true;
	}

	bool _GSVctrl::link(void)
	{
		IF_F(!this->_JSONbase::link());

		Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_GSVgrid", &n);
        m_pGgrid = (_GSVgrid *)(pK->getInst(n));

		vector<string> vGn;
		pK->a("vGeometryBase", &vGn);
		for (string gn : vGn)
		{
			_GeometryBase* pG = (_GeometryBase *)(pK->getInst(gn));
			IF_CONT(!pG);

			m_vpGB.push_back(pG);
		}

		return true;
	}

	bool _GSVctrl::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);

		IF_F(!m_pT->start(getUpdate, this));
		IF_F(!m_pTr->start(getUpdateR, this));

		return true;
	}

	int _GSVctrl::check(void)
	{
		IF__(!m_pGgrid, -1);

		return this->_JSONbase::check();
	}

	void _GSVctrl::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			send();

			m_pT->autoFPSto();
		}
	}

	void _GSVctrl::updateR(void)
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

	void _GSVctrl::handleMsg(const string &str)
	{
		value json;
		IF_(!str2JSON(str, &json));

		object &jo = json.get<object>();
		IF_(!jo["cmd"].is<string>());
		string cmd = jo["cmd"].get<string>();

		if (cmd == "resetTR")
			resetTR(jo);
		else if (cmd == "setT")
			setT(jo);
		else if (cmd == "setR")
			setR(jo);
		else if (cmd == "setSelectedCellIdx")
			setSelectedCellIdx(jo);
		else if (cmd == "saveConfig")
			saveConfig(jo);
		else if (cmd == "getConfig")
			getConfig(jo);
	}

	void _GSVctrl::resetTR(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["_GeometryBase"].is<string>());

		string gName = o["_GeometryBase"].get<string>();
	}

	void _GSVctrl::setT(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["_GeometryBase"].is<string>());
		string gName = o["_GeometryBase"].get<string>();

		IF_(!o["vT"].is<value::array>());
		value::array arr = o["vT"].get<value::array>();
		value::array::iterator it = arr.begin();
		vDouble3 vT;

		IF_(!it->is<double>());
		vT.x = it->get<double>();
		it++;

		IF_(!it->is<double>());
		vT.y = it->get<double>();
		it++;

		IF_(!it->is<double>());
		vT.z = it->get<double>();
		it++;

		_GeometryBase* pG = getGeometry(gName);
		NULL_(pG);
		pG->setTranslation(vT);
		pG->updateTranslationMatrix(false);

	}

	void _GSVctrl::setR(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["_GeometryBase"].is<string>());
		string gName = o["_GeometryBase"].get<string>();

		IF_(!o["vR"].is<value::array>());
		value::array arr = o["vR"].get<value::array>();
		value::array::iterator it = arr.begin();
		vDouble3 vR;

		IF_(!it->is<double>());
		vR.x = it->get<double>();
		it++;

		IF_(!it->is<double>());
		vR.y = it->get<double>();
		it++;

		IF_(!it->is<double>());
		vR.z = it->get<double>();
		it++;

		_GeometryBase* pG = getGeometry(gName);
		NULL_(pG);
		pG->setTranslation(vR);
		pG->updateTranslationMatrix(false);

	}

	void _GSVctrl::setSelectedCellIdx(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["_GeometryBase"].is<string>());
		string gName = o["_GeometryBase"].get<string>();


		IF_(!o["vIdx"].is<value::array>());
		value::array arr = o["vIdx"].get<value::array>();
		value::array::iterator it = arr.begin();
		vInt3 vIdx;

		IF_(!it->is<double>());
		vIdx.x = it->get<double>();
		it++;

		IF_(!it->is<double>());
		vIdx.y = it->get<double>();
		it++;

		IF_(!it->is<double>());
		vIdx.z = it->get<double>();
		it++;



	}

	void _GSVctrl::saveConfig(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["_GeometryBase"].is<string>());
		string gName = o["_GeometryBase"].get<string>();

		//TODO: save calib to file here
		//TODO: save active cells to file here

	}

	void _GSVctrl::getConfig(picojson::object &o)
	{
		IF_(check() < 0);

		sendConfig();
	}

	void _GSVctrl::send(void)
	{
		IF_(check() < 0);

		uint64_t t = getApproxTbootUs();

		if (m_ieSendHB.update(t))
			sendHeartbeat();
	}

	void _GSVctrl::sendHeartbeat(void)
	{
		IF_(check() < 0);

		object o;
		JO(o, "cmd", "hb");
		JO(o, "s", "");
		JO(o, "msg", m_msg);

		m_msg = "";

		sendMsg(o);
	}

	void _GSVctrl::sendConfig(void)
	{
		IF_(check() < 0);

		// m_msg = "Config updated";
		// LivoxAutoScanConfig c = m_pLivox->getConfig();

		// object o;
		// JO(o, "cmd", "getConfig");

		// sendMsg(o);
	}


	_GeometryBase* _GSVctrl::getGeometry(const string& n)
	{
		for(_GeometryBase* pG : m_vpGB)
		{
			string* pN = pG->getName();
			if(*pN == n)
				return pG;
		}

		return NULL;
	}

	void _GSVctrl::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_JSONbase::console(pConsole);

		if (m_pTr)
			m_pTr->console(pConsole);
	}

}
