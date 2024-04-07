#include "_AP_droneBox.h"

namespace kai
{

    _AP_droneBox::_AP_droneBox()
    {
        m_pAP = NULL;
        m_pAPland = NULL;

        m_bAutoArm = false;
        m_altTakeoff = 20.0;
        m_altLand = 20.0;
        m_dLanded = 5;

        m_targetDroneBoxID = -1;
        m_vTargetDroneBoxPos.clear();
    }

    _AP_droneBox::~_AP_droneBox()
    {
    }

    bool _AP_droneBox::init(void *pKiss)
    {
        IF_F(!this->_DroneBoxState::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("bAutoArm", &m_bAutoArm);
        pK->v("altTakeoff", &m_altTakeoff);
        pK->v("altLand", &m_altLand);
        pK->v("dLanded", &m_dLanded);

        pK->v("targetDroneBoxID", &m_targetDroneBoxID);
        pK->v("vTargetDroneBoxPos", &m_vTargetDroneBoxPos);

        return true;
    }

	bool _AP_droneBox::link(void)
	{
		IF_F(!this->_DroneBoxState::link());
		Kiss *pK = (Kiss *)m_pKiss;

        string n;

        n = "";
        pK->v("_AP_base", &n);
        m_pAP = (_AP_base *)(pK->getInst(n));
        IF_Fl(!m_pAP, n + ": not found");

        n = "";
        pK->v("_AP_land", &n);
        m_pAPland = (_AP_land *)(pK->getInst(n));
        IF_Fl(!m_pAPland, n + ": not found");

		return true;
	}

    bool _AP_droneBox::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _AP_droneBox::check(void)
    {
        NULL__(m_pAP, -1);
        NULL__(m_pAP->m_pMav, -1);
        NULL__(m_pAPland, -1);

        return this->_DroneBoxState::check();
    }

    void _AP_droneBox::update(void)
    {
        while (m_pT->bThread())
        {
            m_pT->autoFPSfrom();

            updateDroneBox();

            m_pT->autoFPSto();
        }
    }

    void _AP_droneBox::updateDroneBox(void)
    {
        this->_DroneBoxState::updateDroneBox();
        IF_(check() < 0);

        int apMode = m_pAP->getApMode();
        bool bApArmed = m_pAP->bApArmed();
        float alt = m_pAP->getGlobalPos().w; //relative altitude


        // For manual reset
        if (apMode == AP_COPTER_STABILIZE)
        {
            m_pSC->transit(m_state.STANDBY);
            m_state.update(m_pSC);
        }

        // Standby
        if (m_state.bSTANDBY())
        {
            IF_(apMode != AP_COPTER_GUIDED);

            m_pSC->transit(m_state.TAKEOFF_REQUEST);
            m_state.update(m_pSC);
        }

        // Takeoff procedure
        if (m_state.bTAKEOFF_REQUEST())
        {
            return;
        }

        // Takeoff!
        if (m_state.bTAKEOFF_READY())
        {
            IF_(apMode != AP_COPTER_GUIDED);

            if (m_bAutoArm)
                m_pAP->setApArm(true);

            IF_(!bApArmed);

            m_pAP->m_pMav->clNavTakeoff(m_altTakeoff + 1.0);

            IF_(alt < m_altTakeoff);

            m_pT->sleepT(SEC_2_USEC * 5); //wait to send droneBox to close
            m_pSC->transit(m_state.AIRBORNE);
            m_state.update(m_pSC);
        }

        if (m_state.bAIRBORNE())
        {
            if (apMode == AP_COPTER_GUIDED)
                m_pAP->setApMode(AP_COPTER_AUTO);

            IF_(apMode != AP_COPTER_AUTO);



// TODO:

            IF_(alt > m_altLand);

            m_pSC->transit(m_state.LANDING_REQUEST);
            m_state.update(m_pSC);
        }

        // landing procedure
        if (m_state.bLANDING_REQUEST())
        {
            // hold pos until landing ready
            if (apMode == AP_COPTER_AUTO || apMode == AP_COPTER_RTL)
                m_pAP->setApMode(AP_COPTER_GUIDED);

            return;
        }

        // vision navigated descend
        if (m_state.bLANDING())
        {
            if (apMode == AP_COPTER_AUTO || apMode == AP_COPTER_RTL)
                m_pAP->setApMode(AP_COPTER_GUIDED);

            IF_(!m_pAPland->bComplete());
            IF_(apMode != AP_COPTER_GUIDED);    // for test and debug

            m_pSC->transit(m_state.TOUCHDOWN);
            m_state.update(m_pSC);
        }

        if (m_state.bTOUCHDOWN())
        {
            //switch to AP controlled landing
            if (apMode == AP_COPTER_GUIDED)
                m_pAP->setApMode(AP_COPTER_LAND);

            //check if touched down
            IF_(bApArmed);

            m_pSC->transit(m_state.LANDED);
            m_state.update(m_pSC);
        }

        if (m_state.bLANDED())
        {
            m_pT->sleepT(SEC_2_USEC * m_dLanded);
            m_pSC->transit(m_state.STANDBY);
            m_state.update(m_pSC);
        }
    }

    void _AP_droneBox::landingReady(bool bReady)
    {
        IF_(!bReady);

        if (m_state.bLANDING_REQUEST())
            m_pSC->transit(m_state.LANDING);
    }

    void _AP_droneBox::takeoffReady(bool bReady)
    {
        IF_(!bReady);

        if (m_state.bTAKEOFF_REQUEST())
            m_pSC->transit(m_state.TAKEOFF_READY);
    }

	void _AP_droneBox::addTargetDroneBox(int id, vDouble2 vPdb)
    {
        return; // use fixed id for test first

        vDouble4 v = m_pAP->getGlobalPos();
        vDouble2 vPap;
        vPap.set(v.x, v.y);

        double d = (vPap - m_vTargetDroneBoxPos).len();
        double dNew = (vPap - vPdb).len();
        IF_(d < dNew);

        m_targetDroneBoxID = id;
        m_vTargetDroneBoxPos = vPdb;   
    }

	int _AP_droneBox::getTargetDroneBoxID(void)
    {
        return m_targetDroneBoxID;
    }

}
