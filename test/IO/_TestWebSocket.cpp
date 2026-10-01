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

    bool _TestWebSocket::loadConfig(void)
    {
        IF_F(!this->_TestBase::loadConfig());

        return true;
    }

    bool _TestWebSocket::link(InstanceMgr *pM)
    {
        IF_F(!this->_TestBase::link(pM));
        const json &j = *m_pJ;

        string n = "";
        jKv(j, "_WebSocketServer", n);
        m_pWSserver = (_WebSocketServer *)(pM->findModule(n));
        NULL_F(m_pWSserver);

        return true;
    }

    bool _TestWebSocket::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    bool _TestWebSocket::check(void)
    {
        NULL_F(m_pWSserver);

        return this->_TestBase::check();
    }

    void _TestWebSocket::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            while (!check())
                sleep(1);

            _WebSocket *pWS = m_pWSserver->getClient(0);
            IF_CONT(!pWS);

            BytePacketStream *pStreamIn = pWS->getBytePacketStreamOut();
            BytePacketStream *pStreamOut = pWS->getBytePacketStreamIn();
            IF_CONT(!pStreamIn || !pStreamOut);

            vector<BYTE_PACKET> vBp;
            pStreamIn->getPackets(vBp, m_tLastBpStreamIn);
            for (const BYTE_PACKET &bp : vBp)
            {
                m_tLastBpStreamIn = bp.m_tStamp;
                json j = json::object();
                j["id"] = i2str(1);
                j["cmd"] = "heartbeat";
                j["t"] = li2str(m_pT->getTfromNs());

                string msg = j.dump();
                pStreamOut->addPacket(vector<uint8_t>(msg.begin(), msg.end()));
            }
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
