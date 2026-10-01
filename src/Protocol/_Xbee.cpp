#include "_Xbee.h"

namespace kai
{

    _Xbee::_Xbee()
    {
    }

    _Xbee::~_Xbee()
    {
    }

    bool _Xbee::loadConfig(void)
    {
        IF_F(!this->_ProtocolBase::loadConfig());
        const json &j = *m_pJ;

        string addr = "";
        jKv(j, "myAddr", addr);
        m_myAddr = getAddr(addr);

        return true;
    }

    bool _Xbee::saveConfig(bool bExport)
    {
        IF_F(!_ProtocolBase::saveConfig(false));

        json &j = *m_pJ;
        std::ostringstream address;
        address << std::hex << m_myAddr;
        j["myAddr"] = address.str();

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _Xbee::link(InstanceMgr *pM)
    {
        IF_F(!this->_ProtocolBase::link(pM));

        return true;
    }

    bool _Xbee::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->startThread(getUpdateW, this));
        return m_pTr->startThread(getUpdateR, this);
    }

    bool _Xbee::check(void)
    {
        return this->_ProtocolBase::check();
    }

    void _Xbee::updateW(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            updateMesh();
        }
    }

    void _Xbee::updateMesh(void)
    {
        IF_(!check());
    }

    void _Xbee::send(const string &dest, uint8_t *pB, int nB)
    {
        send(getAddr(dest), pB, nB);
    }

    void _Xbee::send(uint64_t dest, uint8_t *pB, int nB)
    {
        IF_(!check());
        NULL_(pB);
        IF_(nB == 0);

        XBframe_transitRequest f;
        f.m_destAddr = dest;
        f.encode(pB, nB);

        NULL_(m_pBpStreamOut);
        m_pBpStreamOut->addPacket(vector<uint8_t>(f.m_pF, f.m_pF + f.m_nF));
    }

    void _Xbee::updateR(void)
    {
        XBframe xbFrame;
        xbFrame.clear();

        while (m_pTr->bRun())
        {
            if (readFrame(&xbFrame))
            {
                handleFrame(&xbFrame);
                xbFrame.clear();
            }
            else
            {
                m_pTr->autoFPS();
            }
        }
    }

    bool _Xbee::readFrame(XBframe *pF)
    {
        IF_F(!check());
        NULL_F(pF);

        uint8_t b;
        while (readByte(&b))
        {
            bool r = pF->input(b);

            IF__(r, true);
        }

        return false;
    }

    void _Xbee::handleFrame(XBframe *pF)
    {
        NULL_(pF);

        uint8_t fType = pF->m_pB[3];

        if (fType == 0x90)
        {
            // Receive Packet
            XBframe_receivePacket rP;
            IF_(!rP.decode(pF->m_pB, pF->m_iB));

            m_cbReceivePacket.call(rP);
        }
        else if (fType == 0x88)
        {
            // AT command Response
        }
        else if (fType == 0x8A)
        {
            // Modem Status
        }
        else if (fType == 0x8B)
        {
            // Transmit Status
        }
        else if (fType == 0x8D)
        {
            // Route information packet
        }
        else if (fType == 0x8E)
        {
            // Aggregate addressing update
        }
        else if (fType == 0x91)
        {
            // Explicit Rx Indicator
        }
        else if (fType == 0x92)
        {
            // IO Data Sample Rx Indicator
        }
        else if (fType == 0x95)
        {
            // Node Identification Indicator
        }
        else if (fType == 0x97)
        {
            // Remote AT Command Response
        }
    }

    uint64_t _Xbee::getMyAddr(void)
    {
        return m_myAddr;
    }

    uint64_t _Xbee::getAddr(const string &sAddr)
    {
        return strtoull(sAddr.c_str(), NULL, 16);
    }

    bool _Xbee::setCbReceivePacket(CbXBeeReceivePacket pCb, void *pInst)
    {
        NULL_F(pInst);

        m_cbReceivePacket.set(pCb, pInst);
        return true;
    }

    void _Xbee::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ProtocolBase::console(pConsole);

        ((_Console *)pConsole)->addMsg(check() ? "BytePacketStream linked" : "BytePacketStream unavailable", 0);
    }
}
