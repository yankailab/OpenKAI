#include "_DroneBox.h"

namespace kai
{

_DroneBox::_DroneBox()
{
    m_pMB = NULL;
    m_iSlave = 1;
}

_DroneBox::~_DroneBox()
{
}

bool _DroneBox::init ( void* pKiss )
{
    IF_F ( !this->_GCSbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "iSlave", &m_iSlave );

    string n;

    n = "";
    pK->v ( "_Modbus", &n );
    m_pMB = ( _Modbus* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pMB, n + ": not found" );

    return true;
}

bool _DroneBox::start ( void )
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

int _DroneBox::check ( void )
{
    NULL__ ( m_pMB, -1 );
    IF__ ( !m_pMB->bOpen(), -1 );

    return this->_GCSbase::check();
}

void _DroneBox::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_MissionBase::update();

        updateGCS();

        this->autoFPSto();
    }
}

void _DroneBox::updateGCS ( void )
{
    IF_ ( check() <0 );

    this->_GCSbase::updateGCS();
    
    if(m_state.bSTANDBY())
    {
        boxLandingComplete();
    }
    else if(m_state.bTAKEOFF_REQUEST())
    {
        if( bBoxTakeoffReady())
        {
            m_pMC->transit(m_state.TAKEOFF_READY);
            return;
        }

        boxTakeoffPrepare();
    }
    else if(m_state.bTAKEOFF_READY())
    {
        
    }
    else if(m_state.bAIRBORNE())
    {
        boxTakeoffComplete();
    }    
    else if(m_state.bLANDING_REQUEST())
    {
        if( bBoxLandingReady())
        {
            m_pMC->transit(m_state.LANDING_READY);
            return;
        }

        boxLandingPrepare();
    }
    else if(m_state.bLANDING_READY())
    {
        
    }
}

void _DroneBox::boxLandingPrepare ( void )
{
    IF_ ( check() <0 );

    //01 06 00 00 00 01 48 0A
    m_pMB->writeRegister ( m_iSlave, 0, 1 );
}

bool _DroneBox::bBoxLandingReady ( void )
{
    IF_F ( check() <0 );

    //01 03 00 01 00 01 D5 CA
    uint16_t b;
    int r = m_pMB->readRegisters ( m_iSlave, 1, 1, &b );
    IF_F ( r != 1 );

    return ( b==1 ) ?true:false;
}

void _DroneBox::boxLandingComplete ( void )
{
    IF_ ( check() <0 );

    //01 06 00 02 00 01 E9 CA
    m_pMB->writeRegister ( m_iSlave, 2, 1 );
}

void _DroneBox::boxTakeoffPrepare ( void )
{
    IF_ ( check() <0 );

    //01 06 00 03 00 01 B8 0A
    m_pMB->writeRegister ( m_iSlave, 3, 1 );
}

bool _DroneBox::bBoxTakeoffReady ( void )
{
    IF_F ( check() <0 );

    //01 03 00 04 00 01 C5 CB
    uint16_t b;
    int r = m_pMB->readRegisters ( m_iSlave, 4, 1, &b );
    IF_F ( r != 1 );

    return ( b==1 ) ?true:false;
}

void _DroneBox::boxTakeoffComplete ( void )
{
    IF_ ( check() <0 );

    //01 06 00 05 00 01 58 0B
    m_pMB->writeRegister ( m_iSlave, 5, 1 );
}

void _DroneBox::boxRecover ( void )
{
    IF_ ( check() <0 );

    //01 06 00 06 00 01 A8 0B
    m_pMB->writeRegister ( m_iSlave, 6, 1 );
}

void _DroneBox::draw ( void )
{
    this->_GCSbase::draw();
}

}
