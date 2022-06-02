#include "_AP_gcs.h"

namespace kai
{

    _AP_gcs::_AP_gcs()
    {
        m_pAP = NULL;
        m_bAutoArm = false;
        m_altAirborne = 20.0;
        m_dLanded = 5;
    }

    _AP_gcs::~_AP_gcs()
    {
    }

    bool _AP_gcs::init(void *pKiss)
    {
        IF_F(!this->_GCSbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("bAutoArm", &m_bAutoArm);
        pK->v("altAirborne", &m_altAirborne);
        pK->v("dLanded", &m_dLanded);

        string n;

        n = "";
        pK->v("_AP_base", &n);
        m_pAP = (_AP_base *)(pK->getInst(n));
        IF_Fl(!m_pAP, n + ": not found");

        return true;
    }

    bool _AP_gcs::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _AP_gcs::check(void)
    {
        NULL__(m_pAP, -1);
        NULL__(m_pAP->m_pMav, -1);
        NULL__(m_pSC, -1);

        return this->_GCSbase::check();
    }

    void _AP_gcs::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();
            this->_GCSbase::update();

            updateGCS();

            m_pT->autoFPSto();
        }
    }

    void _AP_gcs::updateGCS(void)
    {
        this->_GCSbase::updateGCS();
        IF_(check() < 0);

        int apMode = m_pAP->getApMode();
        bool bApArmed = m_pAP->bApArmed();
        float alt = m_pAP->getGlobalPos().w; //relative altitude

        // For manual reset
        if (apMode == AP_COPTER_STABILIZE)
        {
            m_pSC->transit(m_state.STANDBY);
            return;
        }

        // Standby
        if (m_state.bSTANDBY())
        {
            IF_(apMode != AP_COPTER_GUIDED);

            m_pSC->transit(m_state.TAKEOFF_REQUEST);
            return;
        }

        // Takeoff procedure
        if (m_state.bTAKEOFF_REQUEST())
        {
            return;
        }

        if (m_state.bTAKEOFF_READY())
        {
            IF_(apMode != AP_COPTER_GUIDED);

            if (m_bAutoArm)
                m_pAP->setApArm(true);

            IF_(!bApArmed);

            m_pAP->m_pMav->clNavTakeoff(m_altAirborne + 1.0);

            if (alt > m_altAirborne)
            {
                m_pT->sleepT(SEC_2_USEC * 5); //wait to send droneBox to close
                m_pSC->transit(m_state.AIRBORNE);
            }

            return;
        }

        if (m_state.bAIRBORNE())
        {
            if (apMode == AP_COPTER_GUIDED)
                m_pAP->setApMode(AP_COPTER_AUTO);

            IF_(apMode != AP_COPTER_AUTO);

            IF_(alt > m_altAirborne);

            m_pSC->transit(m_state.LANDING_REQUEST);
            return;
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

            return;
        }

        if (m_state.bTOUCHDOWN())
        {
            //switch to AP controlled landing
            if (apMode == AP_COPTER_GUIDED)
                m_pAP->setApMode(AP_COPTER_LAND);

            //check if touched down
            IF_(bApArmed);

            m_pSC->transit(m_state.LANDED);
            return;
        }

        if (m_state.bLANDED())
        {
            m_pT->sleepT(SEC_2_USEC * m_dLanded);
            m_pSC->transit(m_state.STANDBY);
            return;
        }
    }

    void _AP_gcs::landingReady(bool bReady)
    {
        IF_(!bReady);

        if (m_state.bLANDING_REQUEST())
            m_pSC->transit(m_state.LANDING);
    }

    void _AP_gcs::takeoffReady(bool bReady)
    {
        IF_(!bReady);

        if (m_state.bTAKEOFF_REQUEST())
            m_pSC->transit(m_state.TAKEOFF_READY);
    }

}
