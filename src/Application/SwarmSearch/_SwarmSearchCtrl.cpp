#include "_SwarmSearchCtrl.h"

namespace kai
{

    _SwarmSearchCtrl::_SwarmSearchCtrl()
    {
        m_pXb = NULL;
        m_pSwarm = NULL;

        m_ieSendHB.init(USEC_1SEC);
    }

    _SwarmSearchCtrl::~_SwarmSearchCtrl()
    {
    }

    bool _SwarmSearchCtrl::init(void *pKiss)
    {
        IF_F(!this->_StateBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("myID", &m_node.m_ID);
        pK->v("tIntSendHB", &m_ieSendHB.m_tInterval);

        return true;
    }

    bool _SwarmSearchCtrl::link(void)
    {
        IF_F(!this->_StateBase::link());
        IF_F(!m_pSC);
        m_state.STANDBY = m_pSC->getStateIdxByName("STANDBY");
        m_state.TAKEOFF = m_pSC->getStateIdxByName("TAKEOFF");
        m_state.AUTO = m_pSC->getStateIdxByName("AUTO");
        m_state.RTL = m_pSC->getStateIdxByName("RTL");
        IF_F(!m_state.bValid());

        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        n = "";
        pK->v("_SwarmSearch", &n);
        m_pSwarm = (_SwarmSearch *)(pK->getInst(n));
        IF_Fl(!m_pSwarm, n + ": not found");

        n = "";
        pK->v("_Xbee", &n);
        m_pXb = (_Xbee *)(pK->getInst(n));
        IF_Fl(!m_pXb, n + ": not found");

        IF_F(!m_pXb->setCbReceivePacket(sOnRecvMsg, this));

        return true;
    }

    bool _SwarmSearchCtrl::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _SwarmSearchCtrl::check(void)
    {
        NULL__(m_pSC, -1);
        NULL__(m_pXb, -1);
        NULL__(m_pSwarm, -1);

        return this->_StateBase::check();
    }

    void _SwarmSearchCtrl::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();
            this->_StateBase::update();

            send();

            m_pT->autoFPSto();
        }
    }

    void _SwarmSearchCtrl::send(void)
    {
        IF_(check() < 0);

        uint64_t t = m_pT->getTfrom();

        if (m_ieSendHB.update(t))
            sendHB();
    }

    void _SwarmSearchCtrl::sendHB(void)
    {
        SWMSG_HB m;
        m.m_srcID = m_node.m_ID;
        m.m_lat = m_node.m_pos.x * 1e7;
        m.m_lng = m_node.m_pos.y * 1e7;
        m.m_alt = m_node.m_alt * 1e2;

        uint8_t pB[XB_N_PAYLOAD];
        int nB = m.encode(pB, XB_N_PAYLOAD);
        IF_(nB <= 0);

        m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
    }

    void _SwarmSearchCtrl::sendSetState(void)
    {
        SWMSG_CMD_SETSTATE m;
        m.m_srcID = m_node.m_ID;
        m.m_dstID = 0;
        m.m_state = m_state.m_iState;

        uint8_t pB[XB_N_PAYLOAD];
        int nB = m.encode(pB, XB_N_PAYLOAD);
        IF_(nB <= 0);

        m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
    }

    void _SwarmSearchCtrl::sendGCupdate(void)
    {
        SWMSG_GC_UPDATE m;
        m.m_srcID = m_node.m_ID;
        m.m_dstID = 0;
        m.m_iGC = 0;
        m.m_w = 1;

        uint8_t pB[XB_N_PAYLOAD];
        int nB = m.encode(pB, XB_N_PAYLOAD);
        IF_(nB <= 0);

        m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
    }

    void _SwarmSearchCtrl::onRecvMsg(const XBframe_receivePacket &d)
    {
        uint8_t mType = d.m_pD[0];

        switch (mType)
        {
        case swMsg_hB:
        {
            SWMSG_HB mHb;
            mHb.m_srcNetAddr = d.m_srcAddr;
            if (mHb.decode(d.m_pD, d.m_nD))
                m_pSwarm->handleMsgHB(mHb);
        }
        break;
        case swMsg_cmd_setState:
        {
            SWMSG_CMD_SETSTATE mSS;
            mSS.m_srcNetAddr = d.m_srcAddr;
            if (mSS.decode(d.m_pD, d.m_nD))
                handleMsgSetState(mSS);
        }
        break;
        case swMsg_gc_update:
        {
            SWMSG_GC_UPDATE mGU;
            mGU.m_srcNetAddr = d.m_srcAddr;
            if (mGU.decode(d.m_pD, d.m_nD))
                m_pSwarm->handleMsgGCupdate(mGU);
        }
        break;
        }
    }

    void _SwarmSearchCtrl::handleMsgSetState(const SWMSG_CMD_SETSTATE &m)
    {
        IF_(check() < 0);
        IF_((m.m_dstID != XB_BRDCAST_ADDR) && (m.m_dstID != m_pXb->getMyAddr()));

        // //TODO: enable iMsg counter

        switch (m.m_state)
        {
        case swMsg_state_standBy:
            m_pSC->transit(m_state.STANDBY);
            break;
        case swMsg_state_takeOff:
            m_pSC->transit(m_state.TAKEOFF);
            break;
        case swMsg_state_auto:
            m_pSC->transit(m_state.AUTO);
            break;
        case swMsg_state_RTL:
            m_pSC->transit(m_state.RTL);
            break;
        case swMsg_state_land:
            m_pSC->transit(m_state.RTL);
            break;
        }
    }

    void _SwarmSearchCtrl::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_StateBase::console(pConsole);
    }

}
