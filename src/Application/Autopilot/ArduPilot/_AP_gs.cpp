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
    
    
    

    int m = m_pMC->getMissionIdx();

    if(m == m_state.STANDBY)
    {
        
    }
    else if(m == m_state.LANDING_REQUEST)
    {
    }
    else if(m == m_state.LANDING_READY)
    {
        
    }
    else if(m == m_state.LANDING_COMPLETE)
    {
    }
    else if(m == m_state.TAKEOFF_REQUEST)
    {
    }
    else if(m == m_state.TAKEOFF_READY)
    {
        
    }
    else if(m == m_state.TAKEOFF_COMPLETE)
    {
    }    
}

void _AP_gs::draw ( void )
{
    this->_MissionBase::draw();
    drawActive();

    addMsg ( "State=" + i2str ( m_iState ) );
}

}
