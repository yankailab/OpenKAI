#include "_Xbee.h"

namespace kai
{

    _Xbee::_Xbee()
    {
        m_myAddr = 0;
    }

    _Xbee::~_Xbee()
    {
    }

    bool _Xbee::init(const json& j)
    {
        IF_F(!this->_ProtocolBase::init(j));
        Kiss *pK = (Kiss *)pKiss;

        string addr = "";
        addr = j.value("myAddr", "");
        m_myAddr = getAddr(addr);

        return true;
    }

    bool _Xbee::link(const json& j, ModuleMgr* pM)
    {
        IF_F(!this->_ProtocolBase::link(j, pM));

        return true;
    }

    bool _Xbee::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    bool _Xbee::check(void)
    {
        return this->_ProtocolBase::check();
    }

    void _Xbee::updateW(void)
    {
        while (m_pT->bAlive())
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

        m_pIO->write(f.m_pF, f.m_nF);
    }

    void _Xbee::updateR(void)
    {
  		XBframe xbFrame;
        xbFrame.clear();

        while (m_pTr->bAlive())
        {
            if (readFrame(&xbFrame))
            {
                handleFrame(&xbFrame);
                xbFrame.clear();
            }
        }
    }

    bool _Xbee::readFrame(XBframe* pF)
    {
        IF_F(!check());
		NULL_F(pF);

		if (m_nRead == 0)
		{
			m_nRead = m_pIO->read(m_pBuf, PB_N_BUF);
			IF_F(m_nRead <= 0);
			m_iRead = 0;
		}

		while (m_iRead < m_nRead)
		{
			bool r = pF->input(m_pBuf[m_iRead++]);
			if (m_iRead == m_nRead)
			{
				m_iRead = 0;
				m_nRead = 0;
			}

			IF__(r, true);
		}

		return false;
    }

    void _Xbee::handleFrame(XBframe* pF)
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
        return strtol(sAddr.c_str(), NULL, 16);
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

        string msg;
        if (m_pIO->bOpen())
            msg = "Connected";
        else
            msg = "Not connected";

        ((_Console *)pConsole)->addMsg(msg, 0);
    }
}
