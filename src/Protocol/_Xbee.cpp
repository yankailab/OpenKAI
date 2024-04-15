#include "_Xbee.h"

namespace kai
{

    _Xbee::_Xbee()
    {
        m_myAddr = 0;
        m_pTr = NULL;
        m_pIO = NULL;
        m_fRecv.reset();
    }

    _Xbee::~_Xbee()
    {
        DEL(m_pTr);
    }

    bool _Xbee::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;
    	

        string addr = "";
        pK->v("myAddr", &addr);
        m_myAddr = getAddr(addr);

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

    bool _Xbee::link(void)
    {
        IF_F(!this->_ModuleBase::link());

        Kiss *pK = (Kiss *)m_pKiss;
        string n;
        n = "";

        F_ERROR_F(pK->v("_IObase", &n));
        m_pIO = (_IObase *)(pK->getInst(n));
        NULL_Fl(m_pIO, n + ": not found");

        return true;
    }

    bool _Xbee::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    int _Xbee::check(void)
    {
        NULL__(m_pTr, -1);
        NULL__(m_pIO, -1);
        IF__(!m_pIO->bOpen(), -1);

        return this->_ModuleBase::check();
    }

    void _Xbee::updateW(void)
    {
        while (m_pT->bAlive())
        {
            if (!m_pIO)
            {
                m_pT->sleepT(SEC_2_USEC);
                continue;
            }

            m_pT->autoFPSfrom();

            updateMesh();

            m_pT->autoFPSto();
        }
    }

    void _Xbee::updateMesh(void)
    {
        IF_(check() < 0);
    }

    void _Xbee::send(const string &dest, uint8_t *pB, int nB)
    {
        send(getAddr(dest), pB, nB);
    }

    void _Xbee::send(uint64_t dest, uint8_t *pB, int nB)
    {
        IF_(check() < 0);
        NULL_(pB);
        IF_(nB == 0);

        XBframe_transitRequest f;
        f.m_destAddr = dest;
        f.encode(pB, nB);

        m_pIO->write(f.m_pF, f.m_nF);
    }

    void _Xbee::updateR(void)
    {
        while (m_pTr->bAlive())
        {
            m_pTr->autoFPSfrom();

            if (recv())
            {
                handleFrame();
                m_fRecv.reset();
            }
            //        m_pT->sleepT ( 0 ); //wait for the IObase to wake me up when received data

            m_pTr->autoFPSto();
        }
    }

    bool _Xbee::recv()
    {
        IF_F(check() < 0);

        uint8_t b;
        int nB;
        while ((nB = m_pIO->read(&b, 1)) > 0)
        {
            if (m_fRecv.m_iB > 0)
            {
                m_fRecv.m_pB[m_fRecv.m_iB] = b;
                m_fRecv.m_iB++;

                if (m_fRecv.m_iB == 3)
                {
                    m_fRecv.m_length = (m_fRecv.m_pB[1] << 8) | m_fRecv.m_pB[2];
                }

                IF_T(m_fRecv.m_iB == m_fRecv.m_length + 4);
            }
            else if (b == XB_DELIM)
            {
                m_fRecv.m_pB[0] = b;
                m_fRecv.m_iB = 1;
                m_fRecv.m_length = 0;
            }
        }

        return false;
    }

    void _Xbee::handleFrame(void)
    {
        uint8_t fType = m_fRecv.m_pB[3];

        if (fType == 0x90)
        {
            // Receive Packet
    		XBframe_receivePacket rP;
            IF_(!rP.decode(m_fRecv.m_pB, m_fRecv.m_iB));

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
        this->_ModuleBase::console(pConsole);

        string msg;
        if (m_pIO->bOpen())
            msg = "Connected";
        else
            msg = "Not connected";

        ((_Console *)pConsole)->addMsg(msg, 0);
    }
}
