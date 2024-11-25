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

    int _TestJSON::init(void *pKiss)
    {
        CHECK_(this->_TestBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        //        pK->v("", &);

        return OK_OK;
    }

    int _TestJSON::link(void)
    {
        CHECK_(this->_TestBase::link());

        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        n = "";
        pK->v("_JSONbaseSender", &n);
        m_pJsender = (_JSONbase *)(pK->findModule(n));
        NULL__(m_pJsender, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_JSONbaseReceiver", &n);
        m_pJreceiver = (_JSONbase *)(pK->findModule(n));
        NULL__(m_pJreceiver, OK_ERR_NOT_FOUND);

        return OK_OK;
    }

    int _TestJSON::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _TestJSON::check(void)
    {
        NULL__(m_pJsender, OK_ERR_NULLPTR);
        NULL__(m_pJreceiver, OK_ERR_NULLPTR);

        return this->_TestBase::check();
    }

    void _TestJSON::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            while(check() != OK_OK)
                sleep(1);

            object o;
            JO(o, "id", (double)1);
            JO(o, "t", (double)m_pT->getTfrom());
            JO(o, "cmd", "stat");
            JO(o, "stat", "STANDBY");

            m_pJreceiver->sendJson(o);

            m_pJsender->sendJson(o);

            m_pT->autoFPSto();
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
