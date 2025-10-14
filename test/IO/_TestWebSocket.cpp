#include "_TestWebSocket.h"

namespace kai
{

    _TestWebSocket::_TestWebSocket()
    {
        m_pWSserver = nullptr;
    }

    _TestWebSocket::~_TestWebSocket()
    {
    }

    int _TestWebSocket::init(void *pKiss)
    {
        CHECK_(this->_TestBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        return OK_OK;
    }

    int _TestWebSocket::link(void)
    {
        CHECK_(this->_TestBase::link());

        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        n = "";
        pK->v("_WebSocketServer", &n);
        m_pWSserver = (_WebSocketServer *)(pK->findModule(n));
        NULL__(m_pWSserver, OK_ERR_NOT_FOUND);

        return OK_OK;
    }

    int _TestWebSocket::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _TestWebSocket::check(void)
    {
        NULL__(m_pWSserver, OK_ERR_NULLPTR);

        return this->_TestBase::check();
    }

    void _TestWebSocket::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            while (check() != OK_OK)
                sleep(1);

            _WebSocket *pWS = m_pWSserver->getClient(0);
            IF_CONT(!pWS);

            uint8_t pB[512];
            int nB;
            nB = pWS->read(pB, 512);

            IF_CONT(nB <= 0);

            // if(nB > 0)
            //     pWS->write(pB, nB);

            object o;
            JO(o, "id", i2str(1));
            JO(o, "cmd", "heartbeat");
            JO(o, "t", li2str(m_pT->getTfrom()));

            string msg = picojson::value(o).serialize();
            pWS->write((uint8_t*)msg.c_str(), msg.length());
        }
    }

    void _TestWebSocket::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_TestBase::console(pConsole);

        // string msg;
        // ((_Console *)pConsole)->addMsg(msg, 1);
    }

}
