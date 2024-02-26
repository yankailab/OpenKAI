#include "_AP_mission.h"

namespace kai
{

	_AP_mission::_AP_mission()
	{
		m_pAP = NULL;
		m_pAPmove = NULL;
		m_iMission = 0;
		m_bMissionGoing = false;
		m_dS = 1e-6;
		m_nCmdSent = 0;
	}

	_AP_mission::~_AP_mission()
	{
	}

	bool _AP_mission::init(void *pKiss)
	{
		IF_F(!this->_JSONbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
    	pK->m_pInst = this;

        pK->v("dS", &m_dS);

		return true;
	}

	bool _AP_mission::link(void)
	{
		IF_F(!this->_JSONbase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		n = "";
		pK->v("_AP_move", &n);
		m_pAPmove = (_AP_move *)(pK->getInst(n));
		IF_Fl(!m_pAPmove, n + ": not found");

		return true;
	}

	bool _AP_mission::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _AP_mission::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);
		NULL__(m_pAPmove, -1);

		return this->_JSONbase::check();
	}

	void _AP_mission::updateW(void)
	{
		while (m_pT->bThread())
		{
			if (!m_pIO)
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			m_pT->autoFPSfrom();

			updateMission();

			m_pT->autoFPSto();
		}
	}

	void _AP_mission::updateMission(void)
	{
		IF_(check() < 0);

		int apMode = m_pAP->getApMode();
		vDouble4 vP = m_pAP->getGlobalPos();

		IF_(!m_pAP->bApArmed());
		if(!m_bMissionGoing)
		{
			if(apMode != AP_COPTER_RTL)
				m_pAPmove->doReposition(vP);
		}

		AP_MISSION* pM = &m_vMission[m_iMission];
		
		double dX = pM->m_vP.x - vP.x;
		double dY = pM->m_vP.y - vP.y;
		double d = sqrt(dX * dX + dY * dY);
		if(d < m_dS)
		{
			m_iMission++;
			m_nCmdSent = 0;
			if(m_iMission >= m_vMission.size())
			{
				m_iMission = 0;
				m_bMissionGoing = false;
				m_pAP->setApMode(AP_COPTER_RTL);
				return;
			}
		}

		m_nCmdSent++;
		if(m_nCmdSent < 5)
			m_pAPmove->doReposition(pM->m_vP);

		// yaw to PlookAt
	}

	void _AP_mission::send(void)
	{
		IF_(check() < 0);

		if (m_ieSendHB.update(m_pT->getTfrom()))
		{
			//            sendHeartbeat();
		}
	}

	void _AP_mission::updateR(void)
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

	void _AP_mission::handleMsg(string &str)
	{
		value json;
		IF_(!str2JSON(str, &json));

		object &jo = json.get<object>();
		IF_(!jo["cmd"].is<string>());
		string cmd = jo["cmd"].get<string>();

		if (cmd == "heartbeat")
			heartbeat(jo);
		else if (cmd == "stat")
			stat(jo);
		else if (cmd == "missionUpdate")
			missionUpdate(jo);
		else if (cmd == "missionStart")
			missionStart(jo);
		else if (cmd == "missionPause")
			missionPause(jo);
		else if (cmd == "missionResume")
			missionResume(jo);
		else if (cmd == "missionStop")
			missionStop(jo);
	}

	void _AP_mission::heartbeat(picojson::object &o)
	{
		IF_(check() < 0);
	}

	void _AP_mission::stat(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		IF_(!o["stat"].is<string>());

		int vID = o["id"].get<double>();
		string stat = o["stat"].get<string>();
	}

	void _AP_mission::missionUpdate(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());
		IF_(!o["mission"].is<picojson::array>());

		int vID = o["id"].get<double>();
		m_vMission.clear();

		picojson::array &aM = o["mission"].get<picojson::array>();
		for (picojson::array::iterator it = aM.begin(); it != aM.end(); it++)
		{
			picojson::object &oM = it->get<picojson::object>();

			AP_MISSION m;
			m.clear();
			m.m_missionID = oM["missionID"].get<double>();
			m.m_spd = oM["spd"].get<double>();
			m.m_tDelay = oM["tDelay"].get<double>();

			m.m_vP.x = oM["lat"].get<double>();
			m.m_vP.y = oM["lon"].get<double>();
			m.m_vP.z = oM["alt"].get<double>();
			m.m_vPlookAt.x = oM["latLookAt"].get<double>();
			m.m_vPlookAt.y = oM["lonLookAt"].get<double>();
			m.m_vPlookAt.z = oM["altLookAt"].get<double>();

			m_vMission.push_back(m);
		}

		// reply
		object jo;
		JO(jo, "cmd", "missionUpdate");
		if (m_vMission.empty())
		{
			JO(jo, "r", "err");
			JO(jo, "err", "missioin is empty");
		}
		else
		{
			JO(jo, "r", "ok");
		}
		sendMsg(jo);
	}

	void _AP_mission::missionStart(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());

		m_iMission = 0;
		if(!m_vMission.empty())
		{
			m_bMissionGoing = true;
		}

		// reply
		object jo;
		JO(jo, "cmd", "missionStart");
		if (m_vMission.empty())
		{
			JO(jo, "r", "err");
			JO(jo, "err", "mission is empty");
		}
		else
		{
			JO(jo, "r", "ok");
		}
		sendMsg(jo);
	}

	void _AP_mission::missionPause(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());

		m_bMissionGoing = false;

		// reply
		object jo;
		JO(jo, "cmd", "missionPause");
		JO(jo, "r", "paused");
		JO(jo, "iMission", (double)m_iMission);

		sendMsg(jo);
	}

	void _AP_mission::missionResume(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());

		m_bMissionGoing = true;

		// reply
		object jo;
		JO(jo, "cmd", "missionResume");
		JO(jo, "r", "resumed");
		JO(jo, "iMission", (double)m_iMission);

		sendMsg(jo);
	}

	void _AP_mission::missionStop(picojson::object &o)
	{
		IF_(check() < 0);
		IF_(!o["id"].is<double>());

		m_bMissionGoing = false;
		m_iMission = 0;

		// reply
		object jo;
		JO(jo, "cmd", "missionStop");
		JO(jo, "r", "stopped");

		sendMsg(jo);
	}

	void _AP_mission::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_JSONbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iMission = " + i2str(m_iMission), 1);
		pC->addMsg("nMission = " + i2str(m_vMission.size()), 1);
		pC->addMsg("bMissionGoing = " + i2str(m_bMissionGoing), 1);

	}

}
