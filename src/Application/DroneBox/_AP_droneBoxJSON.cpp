#include "_AP_droneBoxJSON.h"

namespace kai
{

    _AP_droneBoxJSON::_AP_droneBoxJSON()
    {
        m_pAPdroneBox = NULL;

        m_ID = -1;
        m_vPos.clear();
    }

    _AP_droneBoxJSON::~_AP_droneBoxJSON()
    {
        DEL(m_pTr);
    }

    int _AP_droneBoxJSON::init(void *pKiss)
    {
        CHECK_(this->_JSONbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        return OK_OK;
    }

	int _AP_droneBoxJSON::link(void)
	{
		CHECK_(this->_JSONbase::link());
		CHECK_(m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_AP_droneBox", &n);
        m_pAPdroneBox = (_AP_droneBox *)(pK->findModule(n));
        NULL__(m_pAPdroneBox, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

    int _AP_droneBoxJSON::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        NULL__(m_pTr, OK_ERR_NULLPTR);
        CHECK_(m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    int _AP_droneBoxJSON::check(void)
    {
        NULL__(m_pAPdroneBox, OK_ERR_NULLPTR);

        return this->_JSONbase::check();
    }

    void _AP_droneBoxJSON::updateW(void)
    {
        while (m_pT->bAlive())
        {
            if (!m_pIO)
            {
                m_pT->sleepT(SEC_2_USEC);
                continue;
            }

            if (!m_pIO->bOpen())
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

    void _AP_droneBoxJSON::send(void)
    {
        IF_(check() != OK_OK);

        this->_JSONbase::send();

        string state = m_pAPdroneBox->getState();

        object o;
        JO(o, "id", (double)1);
        JO(o, "t", (double)m_pT->getTfrom());

        if (state == "STANDBY")
        {
            JO(o, "cmd", "stat");
            JO(o, "stat", "STANDBY");
            sendMsg(o);
            return;
        }

        if (state == "TAKEOFF_REQUEST")
        {
            JO(o, "cmd", "req");
            JO(o, "do", "takeoff");
            sendMsg(o);
            return;
        }

        if (state == "AIRBORNE")
        {
            JO(o, "cmd", "stat");
            JO(o, "stat", "AIRBORNE");
            sendMsg(o);
            return;
        }

        if (state == "LANDING_REQUEST")
        {
            JO(o, "cmd", "req");
            JO(o, "do", "landing");
            sendMsg(o);
            return;
        }

        if (state == "LANDED")
        {
            JO(o, "cmd", "stat");
            JO(o, "stat", "LANDED");
            sendMsg(o);
            return;
        }
    }

    void _AP_droneBoxJSON::updateR(void)
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

    void _AP_droneBoxJSON::handleMsg(string &str)
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
        else if (cmd == "ack")
            ack(jo);
    }

    void _AP_droneBoxJSON::heartbeat(picojson::object &o)
    {
        IF_(check() != OK_OK);
        IF_(!o["id"].is<double>());
        IF_(!o["lat"].is<double>());
        IF_(!o["lng"].is<double>());

        m_ID = o["id"].get<double>();
        m_vPos.x = o["lat"].get<double>();
        m_vPos.y = o["lng"].get<double>();
    }

    void _AP_droneBoxJSON::stat(picojson::object &o)
    {
        IF_(check() != OK_OK);
        IF_(!o["id"].is<double>());
        IF_(!o["stat"].is<string>());

        int vID = o["id"].get<double>();
        string stat = o["stat"].get<string>();

        m_pAPdroneBox->setBoxState(stat);
    }

    void _AP_droneBoxJSON::ack(picojson::object &o)
    {
        IF_(check() != OK_OK);
        IF_(!o["id"].is<double>());
        IF_(!o["do"].is<string>());
        IF_(!o["r"].is<string>());

        string r = o["r"].get<string>();
        bool bReady = (r == "ok");

        string d = o["do"].get<string>();
        if (d == "takeoff")
        {
            m_pAPdroneBox->takeoffReady(bReady);
        }
        else if (d == "landing")
        {
            m_pAPdroneBox->landingReady(bReady);
        }
    }

    void _AP_droneBoxJSON::console(void *pConsole)
    {
        this->_JSONbase::console(pConsole);

        NULL_(m_pTr);
        m_pTr->console(pConsole);
    }

}
