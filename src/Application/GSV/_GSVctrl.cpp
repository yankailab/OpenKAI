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
		IF_Fl(!m_pGgrid, n + ": not found");

		vector<string> vGn;
		pK->a("vGeometryBase", &vGn);
		for (string gn : vGn)
		{
			_GeometryBase *pG = (_GeometryBase *)(pK->getInst(gn));
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

		if (cmd == "updateGrid")
			updateGrid(jo);
		else if (cmd == "setGrid")
			setGrid(jo);
		else if (cmd == "saveGrid")
			saveGrid(jo);
		else if (cmd == "updateTR")
			updateTR(jo);
		else if (cmd == "setTR")
			setTR(jo);
		else if (cmd == "saveGeometryConfig")
			saveGeometryConfig(jo);
		else if (cmd == "setTRall")
			setTRall(jo);
		else if (cmd == "saveGeometryConfigAll")
			saveGeometryConfigAll(jo);
		else if (cmd == "selectCell")
			selectCell(jo);
		else if (cmd == "setCellAlert")
			setCellAlert(jo);
		else if (cmd == "delCellAlert")
			delCellAlert(jo);
		else if (cmd == "saveGridConfig")
			saveGridConfig(jo);
		else if (cmd == "setParams")
			setParams(jo);
		else if (cmd == "autoAlertCells")
			autoAlertCells(jo);
	}

	void _GSVctrl::updateGrid(picojson::object &o)
	{
		IF_(check() < 0);

		vFloat3 vPo = m_pGgrid->getPorigin();
		vFloat3 vCsize = m_pGgrid->getCellSize();
		vInt2 vGx = m_pGgrid->getGridX();
		vInt2 vGy = m_pGgrid->getGridY();
		vInt2 vGz = m_pGgrid->getGridZ();

		object r;
		JO(r, "cmd", "updateGrid");
		JO(r, "pOx", vPo.x);
		JO(r, "pOy", vPo.y);
		JO(r, "pOz", vPo.z);

		JO(r, "vCx", vCsize.x);
		JO(r, "vCy", vCsize.y);
		JO(r, "vCz", vCsize.z);

		JO(r, "vGx", (double)vGx.len());
		JO(r, "vGy", (double)vGy.len());
		JO(r, "vGz", (double)vGz.len());

		sendMsg(r);
	}

	void _GSVctrl::setGrid(picojson::object &o)
	{
		vDouble3 vPo;
		IF_(!o["pOx"].is<double>());
		vPo.x = o["pOx"].get<double>();
		IF_(!o["pOy"].is<double>());
		vPo.y = o["pOy"].get<double>();
		IF_(!o["pOz"].is<double>());
		vPo.z = o["pOz"].get<double>();

		vFloat3 vCsize;
		IF_(!o["vCx"].is<double>());
		vCsize.x = o["vCx"].get<double>();
		IF_(!o["vCy"].is<double>());
		vCsize.y = o["vCy"].get<double>();
		IF_(!o["vCz"].is<double>());
		vCsize.z = o["vCz"].get<double>();

		IF_(!o["vGx"].is<double>());
		int Gx = o["vGx"].get<double>();
		IF_(!o["vGy"].is<double>());
		int Gy = o["vGy"].get<double>();
		IF_(!o["vGz"].is<double>());
		int Gz = o["vGz"].get<double>();

		Gx /= 2;
		Gy /= 2;
		Gz /= 2;

		m_pGgrid->setPorigin(vFloat3(vPo.x, vPo.y, vPo.z));
		m_pGgrid->setGridX(vInt2(-Gx, Gx));
		m_pGgrid->setGridY(vInt2(-Gy, Gy));
		m_pGgrid->setGridZ(vInt2(-Gz, Gz));
		m_pGgrid->setCellSize(vCsize);

		m_pGgrid->initGrid();
	}

	void _GSVctrl::saveGrid(picojson::object &o)
	{
		IF_(check() < 0);

		m_pGgrid->saveConfig();
	}

	void _GSVctrl::updateTR(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["_GeometryBase"].is<string>());
		string gName = o["_GeometryBase"].get<string>();

		_GeometryBase *pG = getGeometry(gName);
		NULL_(pG);

		vDouble3 vT = pG->getTranslation();
		vDouble3 vR = pG->getRotation();

		object r;
		JO(r, "cmd", "updateTR");
		JO(r, "_GeometryBase", gName);
		JO(r, "vTx", vT.x);
		JO(r, "vTy", vT.y);
		JO(r, "vTz", vT.z);
		JO(r, "vRx", vR.x);
		JO(r, "vRy", vR.y);
		JO(r, "vRz", vR.z);

		sendMsg(r);
	}

	void _GSVctrl::setTR(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["_GeometryBase"].is<string>());
		string gName = o["_GeometryBase"].get<string>();

		vDouble3 vT;
		IF_(!o["vTx"].is<double>());
		vT.x = o["vTx"].get<double>();
		IF_(!o["vTy"].is<double>());
		vT.y = o["vTy"].get<double>();
		IF_(!o["vTz"].is<double>());
		vT.z = o["vTz"].get<double>();

		vDouble3 vR;
		IF_(!o["vRx"].is<double>());
		vR.x = o["vRx"].get<double>();
		IF_(!o["vRy"].is<double>());
		vR.y = o["vRy"].get<double>();
		IF_(!o["vRz"].is<double>());
		vR.z = o["vRz"].get<double>();

		_GeometryBase *pG = getGeometry(gName);
		NULL_(pG);
		pG->setTranslation(vT);
		pG->setRotation(vR);
		pG->updateTranslationMatrix(false);

		// IF_(!o["vT"].is<value::array>());
		// value::array arr = o["vT"].get<value::array>();
		// value::array::iterator it = arr.begin();
		// vDouble3 vT;

		// IF_(!it->is<double>());
		// vT.x = it->get<double>();
		// it++;

		// IF_(!it->is<double>());
		// vT.y = it->get<double>();
		// it++;

		// IF_(!it->is<double>());
		// vT.z = it->get<double>();
		// it++;
	}

	void _GSVctrl::saveGeometryConfig(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["_GeometryBase"].is<string>());
		string gName = o["_GeometryBase"].get<string>();

		_GeometryBase *pG = getGeometry(gName);
		NULL_(pG);

		pG->saveConfig();
	}

	void _GSVctrl::setTRall(picojson::object &o)
	{
		vDouble3 vT;
		IF_(!o["vTx"].is<double>());
		vT.x = o["vTx"].get<double>();
		IF_(!o["vTy"].is<double>());
		vT.y = o["vTy"].get<double>();
		IF_(!o["vTz"].is<double>());
		vT.z = o["vTz"].get<double>();

		vDouble3 vR;
		IF_(!o["vRx"].is<double>());
		vR.x = o["vRx"].get<double>();
		IF_(!o["vRy"].is<double>());
		vR.y = o["vRy"].get<double>();
		IF_(!o["vRz"].is<double>());
		vR.z = o["vRz"].get<double>();

		for (_GeometryBase *pG : m_vpGB)
		{
			vDouble3 vTc = pG->getTranslation();
			vDouble3 vRc = pG->getRotation();

			vTc += vT;
			vRc += vR;

			pG->setTranslation(vTc);
			pG->setRotation(vRc);
			pG->updateTranslationMatrix(false);
		}
	}

	void _GSVctrl::saveGeometryConfigAll(picojson::object &o)
	{
		for (_GeometryBase *pG : m_vpGB)
		{
			pG->saveConfig();
		}
	}

	void _GSVctrl::selectCell(picojson::object &o)
	{
		IF_(check() < 0);

		vInt3 vC;

		IF_(!o["vCx"].is<double>());
		vC.x = o["vCx"].get<double>();

		IF_(!o["vCy"].is<double>());
		vC.y = o["vCy"].get<double>();

		IF_(!o["vCz"].is<double>());
		vC.z = o["vCz"].get<double>();

		m_pGgrid->selectCell(vC);
	}

	void _GSVctrl::setCellAlert(picojson::object &o)
	{
		IF_(check() < 0);

		vInt3 vC;

		IF_(!o["vCx"].is<double>());
		vC.x = o["vCx"].get<double>();

		IF_(!o["vCy"].is<double>());
		vC.y = o["vCy"].get<double>();

		IF_(!o["vCz"].is<double>());
		vC.z = o["vCz"].get<double>();

		m_pGgrid->addAlertCell(vC);
	}

	void _GSVctrl::delCellAlert(picojson::object &o)
	{
		IF_(check() < 0);

		vInt3 vC;

		IF_(!o["vCx"].is<double>());
		vC.x = o["vCx"].get<double>();

		IF_(!o["vCy"].is<double>());
		vC.y = o["vCy"].get<double>();

		IF_(!o["vCz"].is<double>());
		vC.z = o["vCz"].get<double>();

		m_pGgrid->deleteAlertCell(vC);
	}

	void _GSVctrl::saveGridConfig(picojson::object &o)
	{
		IF_(check() < 0);

		m_pGgrid->saveConfig();
	}

	void _GSVctrl::setParams(picojson::object &o)
	{
		IF_(check() < 0);

		IF_(!o["nPs"].is<double>());
		int s = o["nPs"].get<double>();

		m_pGgrid->setNpAlertSensitivity(s);
	}

	void _GSVctrl::autoAlertCells(picojson::object &o)
	{
		IF_(check() < 0);

		m_pGgrid->autoFindAlertCells();
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
	}

	_GeometryBase *_GSVctrl::getGeometry(const string &n)
	{
		for (_GeometryBase *pG : m_vpGB)
		{
			string *pN = pG->getName();
			if (*pN == n)
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
