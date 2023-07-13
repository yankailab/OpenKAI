#include "_Xbee.h"

namespace kai
{

    _Xbee::_Xbee()
    {
        m_pTr = NULL;
        m_pIO = NULL;

        m_destAddr = "000000000000FFFF";
    }

    _Xbee::~_Xbee()
    {
        DEL(m_pTr);
    }

    bool _Xbee::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("destAddr", &m_destAddr);

        Kiss *pKt = pK->child("threadR");
        IF_F(pKt->empty());

        m_pTr = new _Thread();
        if (!m_pTr->init(pKt))
        {
            DEL(m_pTr);
            return false;
        }
        pKt->m_pInst = m_pTr;

        return true;
    }

	bool _Xbee::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;
        n = "";

        F_ERROR_F(pK->v("_IOBase", &n));
        m_pIO = (_IOBase *)(pK->getInst(n));
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
        IF__(!m_pIO->isOpen(), -1);

        return this->_ModuleBase::check();
    }

    void _Xbee::updateW(void)
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

    void _Xbee::send(void)
    {
        IF_(check() < 0);

        uint8_t d[6];
        d[0] = 'H';
        d[1] = 'e';
        d[2] = 'l';
        d[3] = 'l';
        d[4] = 'o';
        d[5] = '!';

        XBframe_transitRequest f;
        f.m_destAddr = strtol(m_destAddr.c_str(), NULL, 16); ;
        f.setData(6, d);
        f.encode();

        m_pIO->write(f.m_pF, f.m_nF);
    }

    void _Xbee::updateR(void)
    {
        while (m_pTr->bRun())
        {
            m_pTr->autoFPSfrom();

            if (recv())
            {
//                handleFrame();
            }
            //        m_pT->sleepT ( 0 ); //wait for the IObase to wake me up when received data

            m_pTr->autoFPSto();
        }
    }

    bool _Xbee::recv()
    {
        IF_F(check() < 0);

        uint8_t B;

        while (m_pIO->read(&B, 1) > 0)
        {
            return true;
        }

        return false;
    }

    void _Xbee::decFrame(uint8_t* pB)
    {
        NULL_(pB);


    }

    void _Xbee::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        string msg;
        if (m_pIO->isOpen())
            msg = "Connected";
        else
            msg = "Not connected";

        ((_Console *)pConsole)->addMsg(msg, 0);
    }
}
