#include "_AProver_BR.h"

namespace kai
{

_AProver_BR::_AProver_BR()
{
    m_pAP = NULL;
    m_pD = NULL;

    m_rcMode.init();
    m_rcMode.m_iChan = 8;
    m_rcStickV.init();
    m_rcStickH.init();
}

_AProver_BR::~_AProver_BR()
{
}

bool _AProver_BR::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "iRCmode", &m_rcMode.m_iChan );
    pK->a ( "vRCmode", &m_rcMode.m_vDiv );
    m_rcMode.setup();
    
    pK->v ( "iRCstickV", &m_rcStickV.m_iChan );
    pK->v ( "iRCstickH", &m_rcStickH.m_iChan );    
    m_rcStickV.setup();
    m_rcStickH.setup();    

    IF_F ( !m_pMC );
    IF_F ( !m_iMode.assign ( m_pMC ) );

    string n;
    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pAP, n + ": not found" );

    n = "";
    pK->v ( "Drive", &n );
    m_pD = ( _Drive* ) ( pK->getInst ( n ) );
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

    uint16_t pwm = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
    if ( pwm != UINT16_MAX )
    {
        m_rcMode.pwm ( pwm );

        int iMode = m_rcMode.i();
        switch ( iMode )
        {
        case 0:
            mode = m_iMode.BACKWARD;
            dir = -1.0;
            break;
        case 1:
            mode = m_iMode.MANUAL;
            dir = 0.0;
            break;
        case 2:
            mode = m_iMode.FORWARD;
            dir = 1.0;
            break;
        default:
            mode = m_iMode.MANUAL;
            dir = 0.0;
            break;
        }
    }

    if ( (m_pAP->getApMode() != AP_ROVER_MANUAL) ||
         (!m_pAP->bApArmed()))
    {
        mode = m_iMode.MANUAL;
        dir = 0.0;
        m_pMC->transit ( mode );
        m_pD->setDirection ( dir );
        m_pD->setSpeed ( 0.0 );
        return;
    }
    
    if(mode != m_iMode.MANUAL)
    {
        m_pMC->transit ( mode );
        m_pD->setDirection ( dir );
        return;
    }
    
    //Manual mode
    pwm = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcStickV.m_iChan );
    if( pwm == UINT16_MAX )pwm = m_rcStickV.m_pwmM;
    m_rcStickV.pwm ( pwm );
    
    pwm = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcStickH.m_iChan );
    if( pwm == UINT16_MAX )pwm = m_rcStickH.m_pwmM;
    m_rcStickH.pwm ( pwm );
    
    m_pMC->transit ( mode );
    
    float v = (m_rcStickV.v() - 0.5) * 2.0;
    float d = 0;
    if(v > 0)d=1.0;
    else if(v < 0)d=-1.0;
    
    m_pD->setDirection(d);
    m_pD->setSpeed(abs(v));
    
    v = (m_rcStickH.v() - 0.5) * 2.0;
    v *= d;
    m_pD->setSteering(v);
}

void _AProver_BR::draw ( void )
{
    this->_MissionBase::draw();
    drawActive();

    addMsg ( "rcMode pwm=" + i2str ( m_rcMode.m_pwm ) + ", i=" + i2str ( m_rcMode.i() ) );
    addMsg ( "rcStickV pwm=" + i2str ( m_rcStickV.m_pwm ) + ", v=" + f2str ( m_rcStickV.v() ) );
    addMsg ( "rcStickH pwm=" + i2str ( m_rcStickH.m_pwm ) + ", v=" + f2str ( m_rcStickH.v() ) );
}

}
