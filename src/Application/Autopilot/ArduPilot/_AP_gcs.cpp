#include "_AP_gcs.h"

namespace kai
{

_AP_gcs::_AP_gcs()
{
    m_pAP = NULL;
    m_altAirborne = 20.0;
}

_AP_gcs::~_AP_gcs()
{
}

bool _AP_gcs::init ( void* pKiss )
{
    IF_F ( !this->_GCSbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "altAirborne", &m_altAirborne );

    string n;

    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pAP, n + ": not found" );

    return true;
}

bool _AP_gcs::start ( void )
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AP_gcs::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pSC, -1 );

    return this->_GCSbase::check();
}

void _AP_gcs::update ( void )
{
    while(m_pT->bRun())
    {
        m_pT->autoFPSfrom();
        this->_GCSbase::update();

        updateGCS();

        m_pT->autoFPSto();
    }
}

void _AP_gcs::updateGCS ( void )
{
    IF_ ( check() < 0 );

    this->updateGCS();

    uint32_t apMode = m_pAP->getApMode();
    bool bApArmed = m_pAP->bApArmed();
    float alt = m_pAP->getGlobalPos().w; //relative altitude
    
    
    // For manual reset
    if(apMode == AP_COPTER_STABILIZE)
    {
        m_pSC->transit(m_state.STANDBY);
        return;
    }

    // Standby
    if(m_state.bSTANDBY())
    {
        IF_(apMode != AP_COPTER_AUTO);
        
        m_pSC->transit(m_state.TAKEOFF_REQUEST);        
        return;
    }
    
    //Takeoff procedure
    if(m_state.bTAKEOFF_REQUEST())
    {        
        return;
    }

    if(m_state.bTAKEOFF_READY())
    {
//        m_pAP->setApArm(true);
        
        if(bApArmed && alt > m_altAirborne)
            m_pSC->transit(m_state.AIRBORNE);

        return;
    }
    
    if(m_state.bAIRBORNE())
    {
        IF_(alt > m_altAirborne);
        
        //TODO: check current waypoint == last waypoing?

        m_pSC->transit(m_state.LANDING_REQUEST);
        return;
    }
    
    //Landing procedure
    if(m_state.bLANDING_REQUEST())
    {
        return;
    }

    if(m_state.bLANDING_READY())
    {
        m_pSC->transit(m_state.LANDING);
        return;
    }
    
    if(m_state.bLANDING())
    {
        IF_(bApArmed);
        
        m_pSC->transit(m_state.LANDED);        
        return;        
    }

    if(m_state.bLANDED())
    {
        //add some delay?
        m_pSC->transit(m_state.STANDBY);        
        return;        
    }
    
}

void _AP_gcs::landingReady(bool bReady)
{
    IF_(!bReady);

    if(m_state.bLANDING_REQUEST())
        m_pSC->transit(m_state.LANDING_READY);
}

void _AP_gcs::takeoffReady(bool bReady)
{
    IF_(!bReady);
    
    if(m_state.bTAKEOFF_REQUEST())
        m_pSC->transit(m_state.TAKEOFF_READY);
}

void _AP_gcs::draw ( void )
{
    this->_GCSbase::draw();
}

}
