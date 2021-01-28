#include "_AP_gs.h"

namespace kai
{

_AP_gs::_AP_gs()
{
    m_pAP = NULL;
}

_AP_gs::~_AP_gs()
{
}

bool _AP_gs::init ( void* pKiss )
{
    IF_F ( !this->_GCSbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    string n;

    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pAP, n + ": not found" );

    return true;
}

bool _AP_gs::start ( void )
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

int _AP_gs::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pMC, -1 );

    return this->_MissionBase::check();
}

void _AP_gs::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_MissionBase::update();

        updateGS();

        this->autoFPSto();
    }
}

void _AP_gs::updateGS ( void )
{
    IF_ ( check() <0 );
    
    uint32_t apMode = m_pAP->getApMode();
    bool bApArmed = m_pAP->bApArmed();
    float alt = m_pAP->getGlobalPos().w;
    int iState = m_pMC->getMissionIdx();
    
    if(apMode == AP_COPTER_LOITER)
    {
        if(iState == m_state.STANDBY)
        {
            m_pMC->transit(m_state.TAKEOFF_REQUEST);
        }
    }
    else if(apMode == AP_COPTER_AUTO && bApArmed)
    {
        if(alt > 10.0)
        {
            m_pMC->transit(m_state.AIRBORNE);            
        }
        else
        {
            m_pMC->transit(m_state.LANDING_REQUEST);
        }
    }
    
}

void _AP_gs::getState(int* pState, GCS_STATE* pGstate)
{
    IF_(check()<0);
    NULL_(pState);
    NULL_(pGstate);
    
    *pState = m_pMC->getMissionIdx();
    *pGstate = m_state;
}

void _AP_gs::landingReady(bool bReady)
{
    IF_(!bReady);
    
    int iState = m_pMC->getMissionIdx();
    IF_(iState != m_state.LANDING_REQUEST);
    
    m_pMC->transit(m_state.LANDING_READY);
}

void _AP_gs::takeoffReady(bool bReady)
{
    IF_(!bReady);
    
    int iState = m_pMC->getMissionIdx();
    IF_(iState != m_state.TAKEOFF_REQUEST);
    
    m_pMC->transit(m_state.TAKEOFF_READY);
}

void _AP_gs::draw ( void )
{
    this->_GCSbase::draw();
    drawActive();
}

}
