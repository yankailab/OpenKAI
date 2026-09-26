#include "_TestJSON.h"

namespace kai
{

    _TestJSON::_TestJSON()
    {
        m_pJsender = nullptr;
        m_pJreceiver = nullptr;
    }

    _TestJSON::~_TestJSON()
    {
    }

    bool _TestJSON::loadConfig(void)
    {
        IF_F(!this->_TestBase::loadConfig());

        // vector<Vector4i> vTest;
        // pK->a("avTest", &vTest);

        return true;
    }

    bool _TestJSON::link(void)
    {
        IF_F(!this->_TestBase::link());
        const json &j = *m_pJ;

        string n;

        n = "";
        jKv(j, "_JSONbaseSender", n);
        m_pJsender = (_JSONbase *)(m_pM->findModule(n));
        NULL_F(m_pJsender);

        n = "";
        jKv(j, "_JSONbaseReceiver", n);
        m_pJreceiver = (_JSONbase *)(m_pM->findModule(n));
        NULL_F(m_pJreceiver);

        return true;
    }

    bool _TestJSON::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    bool _TestJSON::check(void)
    {
        NULL_F(m_pJsender);
        NULL_F(m_pJreceiver);

        return this->_TestBase::check();
    }

    void _TestJSON::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            while (!check())
                sleep(1);

            object o;
            JO(o, "id", (double)1);
            JO(o, "t", (double)m_pT->getTfromNs());
            JO(o, "cmd", "stat");
            JO(o, "stat", "STANDBY");

            m_pJreceiver->sendJson(o);

            m_pJsender->sendJson(o);
        }
    }

    void _TestJSON::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_TestBase::console(pConsole);

        // string msg;
        // ((_Console *)pConsole)->addMsg(msg, 1);
    }

}
