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
    m_bThreadON = true;
    int retCode = pthread_create ( &m_threadID, 0, getUpdateThread, this );
    if ( retCode != 0 )
    {
        LOG ( ERROR ) << "Return code: "<< retCode;
        m_bThreadON = false;
        return false;
    }

    return true;
}

int _AP_gcs::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pSC, -1 );

    return this->_GCSbase::check();
}

void _AP_gcs::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_GCSbase::update();

        updateGCS();

        this->autoFPSto();
    }
}

void _AP_gcs::updateGCS ( void )
{
    IF_ ( check() <0 );
    
    this->_GCSbase::updateGCS();
    
    uint32_t apMode = m_pAP->getApMode();
    bool bApArmed = m_pAP->bApArmed();
    float alt = m_pAP->getGlobalPos().w; //relative altitude

    if(apMode == AP_COPTER_LOITER)
    {
        if(!m_state.bTAKEOFF_READY())
        {
            m_pSC->transit(m_state.TAKEOFF_REQUEST);
        }
    }
    else if(bApArmed)
    {
        if(alt > m_altAirborne)
        {
            m_pSC->transit(m_state.AIRBORNE);            
        }
        else if(!m_state.bLANDING_READY())
        {
            m_pSC->transit(m_state.LANDING_REQUEST);
        }
    }
    else
    {
        m_pSC->transit(m_state.STANDBY);        
    }
    
}

void _AP_gcs::landingReady(bool bReady)
{
    IF_(!bReady);
    IF_(!m_state.bLANDING_READY());
    
    m_pSC->transit(m_state.LANDING_READY);
}

void _AP_gcs::takeoffReady(bool bReady)
{
    IF_(!bReady);
    IF_(!m_state.bTAKEOFF_READY());
    
    m_pSC->transit(m_state.TAKEOFF_READY);
}

void _AP_gcs::draw ( void )
{
    this->_GCSbase::draw();
}

}
