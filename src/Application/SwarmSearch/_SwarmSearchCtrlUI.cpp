#include "_SwarmSearchCtrlUI.h"

namespace kai
{

    _SwarmSearchCtrlUI::_SwarmSearchCtrlUI()
    {
        m_Tr = NULL;
    }

    _SwarmSearchCtrlUI::~_SwarmSearchCtrlUI()
    {
        DEL(m_pTr);
    }

    bool _SwarmSearchCtrlUI::init(void *pKiss)
    {
        IF_F(!this->_JSONbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        Kiss *pKt = pK->child("threadR");
        IF_F(pKt->empty());

        m_pTr = new _Thread();
        if (!m_pTr->init(pKt))
        {
            DEL(m_pTr);
            return false;
        }

        return true;
    }

	bool _SwarmSearchCtrlUI::link(void)
	{
		IF_F(!this->_JSONbase::link());
		IF_F(!m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        // pK->v("_DroneBox", &n);
        // m_pDB = (_DroneBox *)(pK->getInst(n));
        // IF_Fl(!m_pDB, n + ": not found");

		return true;
	}

    bool _SwarmSearchCtrlUI::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    int _SwarmSearchCtrlUI::check(void)
    {
//        NULL__(m_pDB, -1);

        return this->_JSONbase::check();
    }

    void _SwarmSearchCtrlUI::updateW(void)
    {
        while (m_pT->bRun())
        {
            if (!m_pIO)
            {
                m_pT->sleepT(SEC_2_USEC);
                continue;
            }

            if (!m_pIO->isOpen())
            {
                if (!m_pIO->open())
                {
                    m_pT->sleepT(SEC_2_USEC);
                    continue;
                }
            }

            m_pT->autoFPSfrom();

            send();

            m_pT->autoFPSto();
        }
    }

    void _SwarmSearchCtrlUI::send(void)
    {
        IF_(check() < 0);

        if (m_tIntHeartbeat.update(m_pT->getTfrom()))
        {
            sendHeartbeat();
        }
    }

    bool _SwarmSearchCtrlUI::sendHeartbeat(void)
    {
        // vDouble2 vP = m_pDB->getPos();

        // object o;
        // JO(o, "id", i2str(m_pDB->getID()));
        // JO(o, "cmd", "heartbeat");
        // JO(o, "t", li2str(m_pT->getTfrom()));
        // JO(o, "lat", lf2str(vP.x,10));
        // JO(o, "lng", lf2str(vP.y,10));

        // return sendMsg(o);
        return true;
    }

    void _SwarmSearchCtrlUI::updateR(void)
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

    void _SwarmSearchCtrlUI::handleMsg(string &str)
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
        else if (cmd == "req")
            req(jo);
    }

    void _SwarmSearchCtrlUI::heartbeat(picojson::object &o)
    {
        IF_(check() < 0);
    }

    void _SwarmSearchCtrlUI::stat(picojson::object &o)
    {
        IF_(check() < 0);
        IF_(!o["id"].is<double>());
        IF_(!o["stat"].is<string>());

        int vID = o["id"].get<double>();
        string stat = o["stat"].get<string>();

    }

    void _SwarmSearchCtrlUI::req(picojson::object &o)
    {
        // IF_(check() < 0);
        // IF_(!o["id"].is<double>());
        // IF_(!o["do"].is<string>());

        // int vID = o["id"].get<double>();
        // string d = o["do"].get<string>();

        // object jo;
        // JO(jo, "id", (double)m_pDB->getID());
        // JO(jo, "cmd", "ack");
        // JO(jo, "do", d);

        // if (d == "takeoff")
        // {
        //     if (m_pDB->takeoffRequest(vID))
        //     {
        //         JO(jo, "r", "ok");
        //     }
        //     else
        //     {
        //         JO(jo, "r", "denied");
        //     }
        // }
        // else if (d == "landing")
        // {
        //     if (m_pDB->landingRequest(vID))
        //     {
        //         JO(jo, "r", "ok");
        //     }
        //     else
        //     {
        //         JO(jo, "r", "denied");
        //     }
        // }

        // sendMsg(jo);
    }

	// void _SwarmSearchCtrlUI::updateVehicles(void)
	// {
	// 	string h = "";

	// 	for(int i=0; i<m_vpAP.size(); i++)
	// 	{
	// 		_AP_base* pAP = m_vpAP[i];
			
	// 		h += "<tr>";
    //         h += "<th scope=\"row\">"+ i2str(i) +"</th>";
    //         h += "<td>"+ pAP->getApModeName() +"</td>";

	// 		vFloat3 vAtt = pAP->getApAttitude();
    //         h += "<td>";
	// 		h += f2str(vAtt.x) + ", ";
	// 		h += f2str(vAtt.y) + ", ";
	// 		h += f2str(vAtt.z);
	// 		h += "</td>";

	// 		float hdg = pAP->getApHdg();
    //         h += "<td>"+ f2str(hdg) +"</td>";

	// 		vDouble4 vGpos = pAP->getGlobalPos();
    //         h += "<td>"+ f2str(vGpos.w) +"</td>";

	// 		float batt = pAP->getBattery();
    //         h += "<td>"+ f2str(batt) +"</td>";

	// 		h += "</tr>";
	// 	}
	// }


	void _SwarmSearchCtrlUI::onRecvMsg(const XBframe_receivePacket &d)
	{
		uint8_t mType = d.m_pD[0];

		// switch (mType)
		// {
		// case swMsg_hB:
		// 	SWMSG_HB mHb;
		// 	if (mHb.decode(d.m_pD, d.m_nD))
		// 		m_pSwarm->handleMsgHB(mHb);
		// 	break;
		// case swMsg_cmd_setState:
		// 	SWMSG_CMD_SETSTATE mSS;
		// 	if (mSS.decode(d.m_pD, d.m_nD))
		// 		handleMsgSetState(mSS);
		// 	break;
		// case swMsg_gc_update:
		// 	SWMSG_GC_UPDATE mGU;
		// 	if (mGU.decode(d.m_pD, d.m_nD))
		// 		m_pSwarm->handleMsgGCupdate(mGU);
		// 	break;
		// }
	}

	void _SwarmSearchCtrlUI::handleMsgSetState(const SWMSG_CMD_SETSTATE &m)
	{
		IF_(check() < 0);
		// IF_((m.m_dstID != 0) && (m.m_dstID != m_pXb->getMyAddr()));

		// //TODO: enable iMsg counter

		// switch (m.m_state)
		// {
		// case swMsg_state_standBy:
		// 	m_pSC->transit(m_state.STANDBY);
		// 	break;
		// case swMsg_state_takeOff:
		// 	m_pSC->transit(m_state.TAKEOFF);
		// 	break;
		// case swMsg_state_auto:
		// 	m_pSC->transit(m_state.AUTO);
		// 	break;
		// case swMsg_state_RTL:
		// 	m_pSC->transit(m_state.RTL);
		// 	break;
		// case swMsg_state_land:
		// 	m_pSC->transit(m_state.RTL);
		// 	break;
		// }
	}


    void _SwarmSearchCtrlUI::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_JSONbase::console(pConsole);

        NULL_(m_pTr);
        m_pTr->console(pConsole);
    }

}
