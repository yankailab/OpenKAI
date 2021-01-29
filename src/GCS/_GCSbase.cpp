#include "_GCSbase.h"

namespace kai
{

_GCSbase::_GCSbase()
{
    m_gcsID = -1;
}

_GCSbase::~_GCSbase()
{
}

bool _GCSbase::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "gcsID", &m_gcsID );

    IF_F ( !m_pMC );
    IF_F ( !m_state.assign ( m_pMC ) );

    string n;

    return true;
}

int _GCSbase::check ( void )
{

    return this->_MissionBase::check();
}

void _GCSbase::updateGCS ( void )
{
    IF_ ( check() <0 );

    m_state.m_iState = m_pMC->getMissionIdx();

    if(m_state.bSTANDBY())
    {       
    }
    else if(m_state.bTAKEOFF_REQUEST())
    {
    }
    else if(m_state.bTAKEOFF_READY())
    {        
    }
    else if(m_state.bAIRBORNE())
    {
    }    
    else if(m_state.bLANDING_REQUEST())
    {
    }
    else if(m_state.bLANDING_READY())
    {        
    }
}

int _GCSbase::getID (void)
{
    return m_gcsID;
}

GCS_STATE* _GCSbase::getState(void)
{    
    return &m_state;
}

bool _GCSbase::landingRequest ( int vID )
{
    IF_F ( check() <0 );
    int m = m_pMC->getMissionIdx();
    
    IF_T(m == m_state.LANDING_READY);
    IF_T(m == m_state.LANDING_REQUEST);
    
    m_pMC->transit(m_state.LANDING_REQUEST);
    
    return true;
}

bool _GCSbase::bLandingReady ( int vID )
{
    IF_F ( check() <0 );
    int m = m_pMC->getMissionIdx();
    
    IF_T(m == m_state.LANDING_READY);

    return false;
}

void _GCSbase::landingStatus ( int vID, bool bComplete )
{
    IF_ ( check() <0 );
    IF_(!bComplete);

    m_pMC->transit(m_state.STANDBY);
}

bool _GCSbase::takeoffRequest ( int vID )
{
    IF_F ( check() <0 );
    int m = m_pMC->getMissionIdx();
    
    IF_T(m == m_state.TAKEOFF_REQUEST);
    IF_T(m == m_state.TAKEOFF_READY);
    
    m_pMC->transit(m_state.TAKEOFF_REQUEST);
    
    return true;
}

bool _GCSbase::bTakeoffReady ( int vID )
{
    IF_F ( check() <0 );
    int m = m_pMC->getMissionIdx();
    
    IF_T(m == m_state.TAKEOFF_READY);

    return false;
}

void _GCSbase::takeoffStatus ( int vID, bool bComplete )
{
    IF_ ( check() <0 );
    IF_(!bComplete);

    m_pMC->transit(m_state.AIRBORNE);
}

void _GCSbase::draw ( void )
{
    this->_MissionBase::draw();
}

}
