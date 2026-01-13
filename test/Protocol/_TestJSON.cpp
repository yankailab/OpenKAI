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

    bool _TestJSON::init(const json& j)
    {
        IF_F(!this->_TestBase::init(j));
        Kiss *pK = (Kiss *)pKiss;

        // vector<vInt4> vTest;
        // pK->a("avTest", &vTest);

        return true;
    }

    bool _TestJSON::link(const json& j, ModuleMgr* pM)
    {
        IF_F(!this->_TestBase::link(j, pM));

        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        n = "";
        n = j.value("_JSONbaseSender", "");
        m_pJsender = (_JSONbase *)(pM->findModule(n));
        NULL__(m_pJsender);

        n = "";
        n = j.value("_JSONbaseReceiver", "");
        m_pJreceiver = (_JSONbase *)(pM->findModule(n));
        NULL__(m_pJreceiver);

        return true;
    }

    bool _TestJSON::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    bool _TestJSON::check(void)
    {
        NULL__(m_pJsender);
        NULL__(m_pJreceiver);

        return this->_TestBase::check();
    }

    void _TestJSON::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            while(check() != OK_OK)
                sleep(1);

            object o;
            JO(o, "id", (double)1);
            JO(o, "t", (double)m_pT->getTfrom());
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
