#include "_AP_droneBox.h"

namespace kai
{

    _AP_droneBox::_AP_droneBox()
    {
        m_pSC = nullptr;
        m_pAP = nullptr;

        m_gpsToutSec = 100;
        m_gpsHaccMax = 1000; //1m uncertainty
        m_tGPSmeasureStart = 0;

        m_bAutoArm = false;
        m_altTakeoff = 20.0;
        m_altLand = 20.0;
    }

    _AP_droneBox::~_AP_droneBox()
    {
    }

    int _AP_droneBox::init(void *pKiss)
    {
        CHECK_(this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("gpsToutSec", &m_gpsToutSec);
        pK->v("gpsHaccMax", &m_gpsHaccMax);
        pK->v("bAutoArm", &m_bAutoArm);
        pK->v("altTakeoff", &m_altTakeoff);
        pK->v("altLand", &m_altLand);

        return OK_OK;
    }

	int _AP_droneBox::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

        string n;

        n = "";
        pK->v("_AP_base", &n);
        m_pAP = (_AP_base *)(pK->findModule(n));
        NULL__(m_pAP, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_StateControl", &n);
        m_pSC = (_StateControl *)(pK->findModule(n));
        NULL__(m_pSC, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

    int _AP_droneBox::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _AP_droneBox::check(void)
    {
        NULL__(m_pSC, OK_ERR_NULLPTR);
        NULL__(m_pAP, OK_ERR_NULLPTR);
        NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);

        return this->_ModuleBase::check();
    }

    void _AP_droneBox::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            updateAPdroneBox();

            m_pT->autoFPSto();
        }
    }

	string _AP_droneBox::getState(void)
    {
        return m_pSC->getCurrentStateName();
    }

    void _AP_droneBox::updateAPdroneBox(void)
    {
        IF_(check() != OK_OK);

        int apMode = m_pAP->getMode();
        bool bApArmed = m_pAP->bApArmed();
        float alt = m_pAP->getGlobalPos().w; //relative altitude

        string state = getState();

        // For manual reset
        if (apMode == AP_COPTER_STABILIZE)
        {
            m_pSC->transit("STANDBY");
            return;
        }

        // Standby
        if (state == "STANDBY")
        {
            IF_(apMode != AP_COPTER_GUIDED);

            m_pSC->transit("TAKEOFF_REQUEST");
            return;
        }

        // Takeoff procedure
        if (state == "TAKEOFF_REQUEST")
        {
            return;
        }

        // Takeoff
        if (state == "TAKEOFF_READY")
        {
            IF_(apMode != AP_COPTER_GUIDED);

            if(m_pAP->getGPSfixType() != GPS_FIX_TYPE_RTK_FIXED ||
                m_pAP->getGPShacc() > m_gpsHaccMax
            )
            {
                if(m_tGPSmeasureStart == 0)
                {
                    m_tGPSmeasureStart = getApproxTbootUs();
                    return;
                }

                IF_(getApproxTbootUs() - m_tGPSmeasureStart < m_gpsToutSec * SEC_2_USEC);

                // GPS fix time out, abandon take-off
                m_pSC->transit("LANDED");
                return;
            }

            m_tGPSmeasureStart = 0;

            if (m_bAutoArm)
                m_pAP->setArm(true);

            IF_(!bApArmed);

            m_pAP->m_pMav->clNavTakeoff(m_altTakeoff + 1.0);

            IF_(alt < m_altTakeoff);

            m_pSC->transit("AIRBORNE");
            return;
        }

        if (state == "AIRBORNE")
        {
            IF_(m_boxState != "AIRBORNE"); // waiting box to close

            if (apMode == AP_COPTER_GUIDED)
                m_pAP->setMode(AP_COPTER_AUTO);

            IF_(apMode != AP_COPTER_AUTO);

            IF_(alt > m_altLand);

            m_pSC->transit("LANDING_REQUEST");
            return;
        }

        // landing procedure
        if (state == "LANDING_REQUEST")
        {
            // hold pos until landing ready
            if (apMode == AP_COPTER_AUTO || apMode == AP_COPTER_RTL)
            {
                m_pAP->setMode(AP_COPTER_GUIDED);
            }

            return;
        }

        // vision navigated descend
        if (state == "LANDING")
        {
            // IF_(!m_pAPland->bComplete());
            IF_(bApArmed);

            m_pSC->transit("TOUCHDOWN");
            return;
        }

        if (state == "TOUCHDOWN")
        {
            //check if touched down
            IF_(bApArmed);

            m_pSC->transit("LANDED");
            return;
        }

        if (state == "LANDED")
        {
            IF_(m_boxState != "LANDED"); // waiting box to close

            m_pSC->transit("STANDBY");
            return;
        }
    }

    void _AP_droneBox::landingReady(bool bReady)
    {
        IF_(check() != OK_OK);
        IF_(!bReady);

        if (getState() == "LANDING_REQUEST")
            m_pSC->transit("LANDING");
    }

    void _AP_droneBox::takeoffReady(bool bReady)
    {
        IF_(check() != OK_OK);
        IF_(!bReady);

        if (getState() == "TAKEOFF_REQUEST")
            m_pSC->transit("TAKEOFF_READY");
    }

	void _AP_droneBox::setBoxState(const string& s)
    {
        m_boxState = s;
    }

}
