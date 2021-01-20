#include "_DroneBox.h"

namespace kai
{

_DroneBox::_DroneBox()
{
    m_pJvehicle = NULL;
    m_pMB = NULL;
}

_DroneBox::~_DroneBox()
{
}

bool _DroneBox::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

//    pK->v ( "iRCstickV", &m_rcStickV.m_iChan );

    IF_F ( !m_pMC );
    IF_F ( !m_sHatch.assign ( m_pMC ) );

    string n;
    n = "";
    pK->v ( "_DroneBoxJSONvehicle", &n );
    m_pJvehicle = ( _DroneBoxJSON* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pJvehicle, n + ": not found" );

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
    NULL__ ( m_pJvehicle, -1 );
    NULL__ ( m_pMB, -1 );

    return this->_MissionBase::check();
}

void _DroneBox::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_MissionBase::update();

        updateBox();

        this->autoFPSto();
    }
}

void _DroneBox::updateBox ( void )
{
    IF_ ( check() <0 );

    float dir = 0.0;
//    int mode = m_iMode.MANUAL;
    
//    m_pMC->transit ( mode );
    
}

void _DroneBox::draw ( void )
{
    this->_MissionBase::draw();
    drawActive();

    addMsg ( "Hatch state=" + i2str ( m_iHatch ) );
    addMsg ( "Platform state=" + i2str ( m_iPlatform ) );
}

}
