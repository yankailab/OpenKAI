#include "_SwarmCtrl.h"

namespace kai
{

    _SwarmCtrl::_SwarmCtrl()
    {
        m_pXb = nullptr;
        m_pSwarm = nullptr;

        m_ieSendHB.init(USEC_1SEC);
        m_ieSendSetState.init(USEC_1SEC);
        m_ieSendGCupdate.init(USEC_1SEC);
    }

    _SwarmCtrl::~_SwarmCtrl()
    {
    }

    int _SwarmCtrl::init(void *pKiss)
    {
        CHECK_(this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;


        pK->v("myID", &m_node.m_id);
        pK->v("ieSendHB", &m_ieSendHB.m_tInterval);
        pK->v("ieSendSetState", &m_ieSendSetState.m_tInterval);
        pK->v("ieSendGCupdate", &m_ieSendGCupdate.m_tInterval);

        return OK_OK;
    }

    int _SwarmCtrl::link(void)
    {
        CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

        n = "";
        pK->v("_StateControl", &n);
        m_pSC = (_StateControl *)(pK->findModule(n));
        NULL__(m_pSC, OK_ERR_NOT_FOUND);

        m_state.STANDBY = m_pSC->getStateIdxByName("STANDBY");
        m_state.TAKEOFF = m_pSC->getStateIdxByName("TAKEOFF");
        m_state.AUTO = m_pSC->getStateIdxByName("AUTO");
        m_state.RTL = m_pSC->getStateIdxByName("RTL");
        IF__(!m_state.bValid(), OK_ERR_INVALID_VALUE);
        m_state.update(m_pSC->getCurrentStateIdx());

        n = "";
        pK->v("_SwarmSearch", &n);
        m_pSwarm = (_SwarmSearch *)(pK->findModule(n));
        NULL__(m_pSwarm, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_Xbee", &n);
        m_pXb = (_Xbee *)(pK->findModule(n));
        NULL__(m_pXb, OK_ERR_NOT_FOUND);

        IF__(!m_pXb->setCbReceivePacket(sOnRecvMsg, this), OK_ERR_UNKNOWN);

        return OK_OK;
    }

    int _SwarmCtrl::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _SwarmCtrl::check(void)
    {
        NULL__(m_pSC, OK_ERR_NULLPTR);
        NULL__(m_pXb, OK_ERR_NULLPTR);
        NULL__(m_pSwarm, OK_ERR_NULLPTR);

        return this->_ModuleBase::check();
    }

    void _SwarmCtrl::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            send();

            m_pT->autoFPSto();
        }
    }

    void _SwarmCtrl::send(void)
    {
        IF_(check() != OK_OK);

        uint64_t t = m_pT->getTfrom();

        // if (m_ieSendHB.update(t))
        //     sendHB();
        if (m_ieSendSetState.update(t))
            sendSetState();
        // if (m_ieSendGCupdate.update(t))
        //     sendGCupdate();
    }

    void _SwarmCtrl::sendHB(void)
    {
        SWMSG_HB m;
        m.m_srcID = m_node.m_id;
        m.m_lat = m_node.m_pos.x * 1e7;
        m.m_lng = m_node.m_pos.y * 1e7;
        m.m_alt = m_node.m_alt * 1e2;

        uint8_t pB[XB_N_PAYLOAD];
        int nB = m.encode(pB, XB_N_PAYLOAD);
        IF_(nB <= 0);

        m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
    }

    void _SwarmCtrl::sendSetState(void)
    {
        SWMSG_CMD_SETSTATE m;
        m.m_srcID = m_node.m_id;
        m.m_dstID = 0;
        m.m_state = m_state.m_iState;

        uint8_t pB[XB_N_PAYLOAD];
        int nB = m.encode(pB, XB_N_PAYLOAD);
        IF_(nB <= 0);

        m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
    }

    void _SwarmCtrl::sendGCupdate(void)
    {
        SWMSG_GC_UPDATE m;
        m.m_srcID = m_node.m_id;
        m.m_dstID = 0;
        m.m_iGC = 0;
        m.m_w = 1;

        uint8_t pB[XB_N_PAYLOAD];
        int nB = m.encode(pB, XB_N_PAYLOAD);
        IF_(nB <= 0);

        m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
    }

	bool _SwarmCtrl::setState(const string& state)
    {
        if(state == "standby")
            m_pSC->transit(m_state.STANDBY);
        else if(state == "takeoff")
            m_pSC->transit(m_state.TAKEOFF);
        else if(state == "auto")
            m_pSC->transit(m_state.AUTO);
        else if(state == "rtl")
            m_pSC->transit(m_state.RTL);

        m_state.update(m_pSC->getCurrentStateIdx());
        return true;
    }

    void _SwarmCtrl::onRecvMsg(const XBframe_receivePacket &d)
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
        case swMsg_gc_update:
        {
            SWMSG_GC_UPDATE mGU;
            mGU.m_srcNetAddr = d.m_srcAddr;
            if (mGU.decode(d.m_pD, d.m_nD))
                m_pSwarm->handleMsgGCupdate(mGU);
        }
        break;
        }

        // The ctrl does not accept state change from nodes?
        // case swMsg_cmd_setState:
        // {
        //     SWMSG_CMD_SETSTATE mSS;
        //     mSS.m_srcNetAddr = d.m_srcAddr;
        //     if (mSS.decode(d.m_pD, d.m_nD))
        //         handleMsgSetState(mSS);
        // }
        // break;

    }

    void _SwarmCtrl::handleMsgSetState(const SWMSG_CMD_SETSTATE &m)
    {
        IF_(check() != OK_OK);
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

        m_state.update(m_pSC->getCurrentStateIdx());
    }

    void _SwarmCtrl::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
    }

}
