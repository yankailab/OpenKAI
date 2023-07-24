#include "_DroneBoxJSON.h"

namespace kai
{

    _DroneBoxJSON::_DroneBoxJSON()
    {
        m_Tr = NULL;
        m_pDB = NULL;
    }

    _DroneBoxJSON::~_DroneBoxJSON()
    {
        DEL(m_pTr);
    }

    bool _DroneBoxJSON::init(void *pKiss)
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

	bool _DroneBoxJSON::link(void)
	{
		IF_F(!this->_JSONbase::link());
		IF_F(!m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_DroneBox", &n);
        m_pDB = (_DroneBox *)(pK->getInst(n));
        IF_Fl(!m_pDB, n + ": not found");

		return true;
	}

    bool _DroneBoxJSON::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    int _DroneBoxJSON::check(void)
    {
        NULL__(m_pDB, -1);

        return this->_JSONbase::check();
    }

    void _DroneBoxJSON::updateW(void)
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

    void _DroneBoxJSON::send(void)
    {
        IF_(check() < 0);

        if (m_ieSendHB.update(m_pT->getTfrom()))
        {
            sendHeartbeat();
        }
    }

    void _DroneBoxJSON::sendHeartbeat(void)
    {
        vDouble2 vP = m_pDB->getPos();

        object o;
        JO(o, "id", i2str(m_pDB->getID()));
        JO(o, "cmd", "heartbeat");
        JO(o, "t", li2str(m_pT->getTfrom()));
        JO(o, "lat", lf2str(vP.x,10));
        JO(o, "lng", lf2str(vP.y,10));

        sendMsg(o);
    }

    void _DroneBoxJSON::updateR(void)
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

    void _DroneBoxJSON::handleMsg(string &str)
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

    void _DroneBoxJSON::heartbeat(picojson::object &o)
    {
        IF_(check() < 0);
    }

    void _DroneBoxJSON::stat(picojson::object &o)
    {
        IF_(check() < 0);
        IF_(!o["id"].is<double>());
        IF_(!o["stat"].is<string>());

        int vID = o["id"].get<double>();
        string stat = o["stat"].get<string>();

        m_pDB->status(vID, stat);
    }

    void _DroneBoxJSON::req(picojson::object &o)
    {
        IF_(check() < 0);
        IF_(!o["id"].is<double>());
        IF_(!o["do"].is<string>());

        int vID = o["id"].get<double>();
        string d = o["do"].get<string>();

        object jo;
        JO(jo, "id", (double)m_pDB->getID());
        JO(jo, "cmd", "ack");
        JO(jo, "do", d);

        if (d == "takeoff")
        {
            if (m_pDB->takeoffRequest(vID))
            {
                JO(jo, "r", "ok");
            }
            else
            {
                JO(jo, "r", "denied");
            }
        }
        else if (d == "landing")
        {
            if (m_pDB->landingRequest(vID))
            {
                JO(jo, "r", "ok");
            }
            else
            {
                JO(jo, "r", "denied");
            }
        }

        sendMsg(jo);
    }

    void _DroneBoxJSON::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_JSONbase::console(pConsole);

        NULL_(m_pTr);
        m_pTr->console(pConsole);
    }

}
