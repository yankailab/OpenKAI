#include "_AProver_BR.h"

namespace kai
{

_AProver_BR::_AProver_BR()
{
    m_pAP = NULL;
    m_pD = NULL;

    m_rcMode.init();
}

_AProver_BR::~_AProver_BR()
{
}

bool _AProver_BR::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "iRCmode", &m_rcMode.m_iChan );
    m_rcMode.setup();

    IF_F ( !m_pMC );
    IF_F ( !m_iMode.assign ( m_pMC ) );

    string n;
    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pAP, n + ": not found" );

    n = "";
    pK->v ( "Drive", &n );
    m_pD = ( Drive* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pD, n + ": not found" );

    return true;
}

bool _AProver_BR::start ( void )
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

int _AProver_BR::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pAP->m_pMav, -1 );
    NULL__ ( m_pMC, -1 );
    NULL__ ( m_pD, -1 );

    return this->_MissionBase::check();
}

void _AProver_BR::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_MissionBase::update();

        updateMode();

        this->autoFPSto();
    }
}

void _AProver_BR::updateMode ( void )
{
    IF_ ( check() <0 );

    float dir = 0.0;
    int mode = m_iMode.MANUAL;

    uint16_t pwmMode = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
    if ( pwmMode != UINT16_MAX )
    {
        m_rcMode.pwm ( pwmMode );

        int iMode = m_rcMode.i();
        switch ( iMode )
        {
        case 0:
            mode = m_iMode.FORWARD;
            dir = 1.0;
            break;
        case 1:
            mode = m_iMode.MANUAL;
            dir = 0.0;
            break;
        case 2:
            mode = m_iMode.BACKWARD;
            dir = -1.0;
            break;
        default:
            mode = m_iMode.MANUAL;
            dir = 0.0;
            break;
        }
    }

    uint32_t apMode = m_pAP->getApMode();
    if ( apMode != AP_ROVER_MANUAL )
    {
        mode = m_iMode.MANUAL;
        dir = 0.0;
    }

    m_pMC->transit ( mode );
    m_pD->setDirection ( dir );
}

void _AProver_BR::draw ( void )
{
    this->_MissionBase::draw();
    drawActive();

    addMsg ( "rcMode pwm=" + i2str ( m_rcMode.m_pwm ) + ", i=" + i2str ( m_rcMode.i() ) );
}

}
