#include "_DroneBox.h"

namespace kai
{

_DroneBox::_DroneBox()
{
    m_pMB = NULL;
    m_iSlave = 1;
    m_lastCMD = dbx_unknown;
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
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _DroneBox::check ( void )
{
    NULL__ ( m_pMB, -1 );
    IF__ ( !m_pMB->bOpen(), -1 );

    return this->_GCSbase::check();
}

void _DroneBox::update ( void )
{
    while(m_pT->bRun())
    {
        m_pT->autoFPSfrom();
        this->_GCSbase::update();

        updateGCS();

        m_pT->autoFPSto();
    }
}

void _DroneBox::updateGCS ( void )
{
    IF_ ( check() <0 );

    this->updateGCS();
    
    
    if(m_state.bSTANDBY())
    {
        boxRecover();
        return;
    }

    
    //Takeoff procedure
    if(m_state.bTAKEOFF_REQUEST())
    {
        boxTakeoffPrepare();

        if( bBoxTakeoffReady())
            m_pSC->transit(m_state.TAKEOFF_READY);
        
        return;
    }

    if(m_state.bTAKEOFF_READY())
    {
        return;
    }
    
    if(m_state.bAIRBORNE())
    {
        boxTakeoffComplete();
        return;
    }
    
    
    //Landing procedure
    if(m_state.bLANDING_REQUEST())
    {
        boxLandingPrepare();

        if( bBoxLandingReady())
            m_pSC->transit(m_state.LANDING_READY);

        return;
    }

    if(m_state.bLANDING_READY())
    {
        return;        
    }
    
    if(m_state.bLANDING())
    {
        return;        
    }

    if(m_state.bLANDED())
    {
        IF_(!boxLandingComplete());
        
        m_pSC->transit(m_state.STANDBY);        
        return;        
    }
}

bool _DroneBox::boxLandingPrepare ( void )
{
    IF_F ( check() <0 );
    IF_T (m_lastCMD == dbx_landingPrepare);

    //01 06 00 00 00 01 48 0A
    int r = m_pMB->writeRegister ( m_iSlave, 0, 1 );
    LOG_I("boxLandingPrepare: " + i2str(r));
    IF_F(r <= 0);
    
    m_lastCMD = dbx_landingPrepare;
    return true;
}

bool _DroneBox::bBoxLandingReady ( void )
{
    IF_F ( check() <0 );

    //01 03 00 01 00 01 D5 CA
    uint16_t b = 0;
    int r = m_pMB->readRegisters ( m_iSlave, 1, 1, &b );
    LOG_I("bBoxLandingReady: " + i2str(r) + ", " + i2str(b));
    IF_F ( r != 1 );
    
    m_lastCMD = dbx_bLandingReady;
    return ( b==1 ) ?true:false;
}

bool _DroneBox::boxLandingComplete ( void )
{
    IF_F ( check() <0 );
    IF_F (m_lastCMD == dbx_landingComplete);

    //01 06 00 02 00 01 E9 CA
    int r = m_pMB->writeRegister ( m_iSlave, 2, 1 );
    LOG_I("boxLandingComplete: " + i2str(r));
    IF_F(r <= 0);
    
    m_lastCMD = dbx_landingComplete;
    return true;
}

bool _DroneBox::boxTakeoffPrepare ( void )
{
    IF_F ( check() <0 );
    IF_T (m_lastCMD == dbx_takeoffPrepare);

    //01 06 00 03 00 01 B8 0A
    int r = m_pMB->writeRegister ( m_iSlave, 3, 1 );
    LOG_I("boxTakeoffPrepare: " + i2str(r));
    IF_F(r <= 0);
    
    m_lastCMD = dbx_takeoffPrepare;
    return true;
}

bool _DroneBox::bBoxTakeoffReady ( void )
{
    IF_F ( check() <0 );

    //01 03 00 04 00 01 C5 CB
    uint16_t b = 0;
    int r = m_pMB->readRegisters ( m_iSlave, 4, 1, &b );
    LOG_I("bBoxTakeoffReady: " + i2str(r) + ", " + i2str(b));
    IF_F ( r != 1 );

    m_lastCMD = dbx_bTakeoffReady;
    return ( b==1 ) ?true:false;
}

bool _DroneBox::boxTakeoffComplete ( void )
{
    IF_F ( check() <0 );
    IF_T (m_lastCMD == dbx_takeoffComplete);

    //01 06 00 05 00 01 58 0B
    int r = m_pMB->writeRegister ( m_iSlave, 5, 1 );
    LOG_I("boxTakeoffComplete: " + i2str(r));
    IF_F ( r <= 0 );

    m_lastCMD = dbx_takeoffComplete;
    return true;
}

bool _DroneBox::boxRecover ( void )
{
    IF_F ( check() <0 );

    //01 06 00 06 00 01 A8 0B
    int r = m_pMB->writeRegister ( m_iSlave, 6, 1 );
    LOG_I("boxRecover: " + i2str(r));
    IF_F( r <= 0 );

    m_lastCMD = dbx_boxRecover;
    return true;
}

void _DroneBox::draw ( void )
{
    this->_GCSbase::draw();
}

}
