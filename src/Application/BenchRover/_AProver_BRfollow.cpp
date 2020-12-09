#include "_AProver_BRfollow.h"

namespace kai
{

_AProver_BRfollow::_AProver_BRfollow()
{
    m_pAP = NULL;
    m_pD = NULL;
    m_pPID = NULL;
    m_pUpath = NULL;
    m_pUtag = NULL;

    m_targetX = 0.5;
    m_pathX = m_targetX;
    m_nSpd = 0.0;
    m_nStr = 0.0;
    m_iTagStop = -1;
    m_iTag = -1;
}

_AProver_BRfollow::~_AProver_BRfollow()
{
}

bool _AProver_BRfollow::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );
    NULL_F( m_pMC );

    Kiss* pK = ( Kiss* ) pKiss;
    pK->v ( "targetX", &m_targetX );
    pK->v ( "nSpd", &m_nSpd );
    pK->v ( "nStr", &m_nStr );
    pK->v ( "iTagStop", &m_iTagStop );

    string n;

    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pAP, n + ": not found" );

    n = "";
    pK->v ( "Drive", &n );
    m_pD = ( _Drive* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pD, n + ": not found" );

    n = "";
    pK->v ( "PID", &n );
    m_pPID = ( PID* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPID, n + " not found" );

    n = "";
    pK->v ( "_UniversePath", &n );
    m_pUpath = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pUpath, n + ": not found" );

    n = "";
    pK->v ( "_UniverseTag", &n );
    m_pUtag = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pUtag, n + ": not found" );

    return true;
}

bool _AProver_BRfollow::start ( void )
{
    m_bThreadON = true;
    int retCode = pthread_create ( &m_threadID, 0, getUpdateThread, this );
    if ( retCode != 0 )
    {
        LOG ( ERROR ) << "Return code: " << retCode;
        m_bThreadON = false;
        return false;
    }

    return true;
}

int _AProver_BRfollow::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pD, -1 );
    NULL__ ( m_pPID, -1 );
    NULL__ ( m_pUpath, -1 );
    NULL__ ( m_pUtag, -1 );

    return this->_MissionBase::check();
}

void _AProver_BRfollow::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_MissionBase::update();

        updateFollow();

        this->autoFPSto();
    }
}

void _AProver_BRfollow::updateFollow ( void )
{
    IF_ ( check() <0 );
	IF_(!bActive());
    
    float dir = m_pD->getDirection();   //+/-1.0
    float nSpd = m_nSpd;
    
    //path following compensation
    _Object* pO = m_pUpath->get(0);
    if ( pO )
        m_pathX = pO->getY();
    else
        m_pathX = m_targetX;

    float errX = dir * (m_pathX - m_targetX);
    m_nStr = dir * m_pPID->update ( errX, 0.0, m_tStamp );
    
    //tag actions
    pO = findTarget();
    if ( pO )
    {
        int iTag = pO->getTopClass();
        if(iTag == m_iTagStop)
        {
            nSpd = 0.0;
            m_nStr = 0.0;
        }
        else if(iTag != m_iTag)
        {
            m_pD->setSteering(0.0);
            m_pD->setSpeed(0.0);
            this->sleepTime(3 * USEC_1SEC);
            m_iTag = iTag;
        }
    }

    m_pD->setSteering(m_nStr);
    m_pD->setSpeed(nSpd);
}

_Object* _AProver_BRfollow::findTarget ( void )
{
    IF_N ( check() <0 );

    _Object *pO;
    _Object *tO = NULL;
    float topY = FLT_MAX;
    int i = 0;
    while ( ( pO = m_pUtag->get ( i++ ) ) != NULL )
    {
        vFloat3 p = pO->getPos();
        IF_CONT ( p.y > topY );

        tO = pO;
        topY = p.x;
    }

    return tO;
}

void _AProver_BRfollow::draw ( void )
{
    this->_MissionBase::draw();
    drawActive();

   	addMsg("nSpd=" + f2str(m_nSpd) + ", nStr=" + f2str(m_nStr));
   	addMsg("pathX=" + f2str(m_pathX) + ", targetX=" + f2str(m_targetX));
}

}
