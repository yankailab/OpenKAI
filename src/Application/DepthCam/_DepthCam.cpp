#include "_DepthCam.h"

namespace kai
{

    _DepthCam::_DepthCam()
    {
        m_bHeartbeat = false;
        m_Tr = NULL;
        m_pRS = NULL;
        m_vBBhb.set(0.4, 0.4, 0.6, 0.6);
        m_pIR = NULL;
    }

    _DepthCam::~_DepthCam()
    {
        DEL(m_pTr);
    }

    bool _DepthCam::init(void *pKiss)
    {
        IF_F(!this->_JSONbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("bHeartbeat", &m_bHeartbeat);
        pK->v("vBBhb", &m_vBBhb);

        string n;
        n = "";
        pK->v("_RealSense", &n);
        m_pRS = (_RealSense *)(pK->getInst(n));
        IF_Fl(!m_pRS, n + ": not found");

        n = "";
        pK->v("_InRange", &n);
        m_pIR = (_InRange *)(pK->getInst(n));
        IF_Fl(!m_pIR, n + ": not found");

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

    bool _DepthCam::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    int _DepthCam::check(void)
    {
        NULL__(m_pRS, -1);

        return this->_JSONbase::check();
    }

    void _DepthCam::updateW(void)
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

    void _DepthCam::send(void)
    {
        IF_(check() < 0);

        if (m_bHeartbeat)
            heartbeat();
    }

    void _DepthCam::heartbeat(void)
    {
        IF_(check() < 0);

        double d = m_pRS->d(m_vBBhb);

        object jo;
        JO(jo, "cmd", "heartBeat");
        JO(jo, "id", (double)-1.0);
        JO(jo, "x", (double)m_vBBhb.x);
        JO(jo, "y", (double)m_vBBhb.y);
        JO(jo, "z", (double)m_vBBhb.z);
        JO(jo, "w", (double)m_vBBhb.w);
        JO(jo, "d", (double)d);

        sendMsg(jo);
    }

    void _DepthCam::updateR(void)
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

    void _DepthCam::handleMsg(string &str)
    {
        value json;
        IF_(!str2JSON(str, &json));

        object &jo = json.get<object>();
        IF_(!jo["cmd"].is<string>());
        string cmd = jo["cmd"].get<string>();

        if (cmd == "detectPos")
            detectPos(jo);
        else if (cmd == "setSensorOption")
            setSensorOption(jo);
        else if (cmd == "setRangeFilter")
            setRangeFilter(jo);
    }

    void _DepthCam::detectPos(picojson::object &o)
    {
        IF_(check() < 0);
        IF_(!o["id"].is<double>());
        IF_(!o["x"].is<double>());
        IF_(!o["y"].is<double>());
        IF_(!o["z"].is<double>());
        IF_(!o["w"].is<double>());

        int bbID = o["id"].get<double>();
        vFloat4 vBB;
        vBB.x = o["x"].get<double>();
        vBB.y = o["y"].get<double>();
        vBB.z = o["z"].get<double>();
        vBB.w = o["w"].get<double>();

        double d = m_pRS->d(vBB);

        object jo;
        JO(jo, "cmd", "detectPos");
        JO(jo, "id", (double)bbID);
        JO(jo, "x", (double)vBB.x);
        JO(jo, "y", (double)vBB.y);
        JO(jo, "z", (double)vBB.z);
        JO(jo, "w", (double)vBB.w);
        JO(jo, "d", (double)d);

        sendMsg(jo);
    }

    void _DepthCam::setSensorOption(picojson::object &o)
    {
        IF_(check() < 0);
        IF_(!o["sensorType"].is<string>());
        IF_(!o["optionID"].is<double>());
        IF_(!o["v"].is<double>());

        string sType = o["sensorType"].get<string>();
        int oID = o["optionID"].get<double>();
        float v = o["v"].get<double>();

        if (sType == "color")
        {
            m_pRS->setCsensorOption((rs2_option)oID, v);
        }
        else if (sType == "depth")
        {
            m_pRS->setDsensorOption((rs2_option)oID, v);
        }
    }

    void _DepthCam::setRangeFilter(picojson::object &o)
    {
        IF_(check() < 0);

        if (o["nHistLevel"].is<double>())
        {
            m_pRS->m_nHistLev = o["nHistLevel"].get<double>();
        }

        if (o["minHistD"].is<double>())
        {
            m_pRS->m_minHistD = o["minHistD"].get<double>();
        }

        if (o["rangeFrom"].is<double>())
        {
            m_pRS->m_vRange.x = o["rangeFrom"].get<double>();
        }

        if (o["rangeTo"].is<double>())
        {
            m_pRS->m_vRange.y = o["rangeTo"].get<double>();
        }

        if (o["filterFrom"].is<double>())
        {
            m_pIR->m_rFrom = o["filterFrom"].get<double>();
        }

        if (o["filterTo"].is<double>())
        {
            m_pIR->m_rTo = o["filterTo"].get<double>();
        }
    }

    void _DepthCam::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_JSONbase::console(pConsole);

        NULL_(m_pTr);
        m_pTr->console(pConsole);
    }

}
