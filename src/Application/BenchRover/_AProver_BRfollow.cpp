#include "_AProver_BRfollow.h"

namespace kai
{

_AProver_BRfollow::_AProver_BRfollow()
{
    m_pAP = NULL;
    m_pD = NULL;
    m_pPIDtagX = NULL;
    m_pPIDtagHdg = NULL;
    m_pU = NULL;

    m_tagTargetX = 0.5;
    m_errX = 0.0;
    m_errHdg = 0.0;
    m_iTagStop = -1;
    m_iTag = -1;
    m_nSpd = 0.0;
    m_nStr = 0.0;

    m_tStop = 3;
}

_AProver_BRfollow::~_AProver_BRfollow()
{
}

bool _AProver_BRfollow::init ( void* pKiss )
{
    IF_F ( !this->_StateBase::init ( pKiss ) );
    NULL_F( m_pSC );

    Kiss* pK = ( Kiss* ) pKiss;
    pK->v ( "targetX", &m_tagTargetX );
    pK->v ( "nSpd", &m_nSpd );
    pK->v ( "nStr", &m_nStr );
    pK->v ( "iTagStop", &m_iTagStop );
    pK->v ( "tStop", &m_tStop );

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
    pK->v ( "PIDtagX", &n );
    m_pPIDtagX = ( PID* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDtagX, n + " not found" );

    n = "";
    pK->v ( "PIDtagHdg", &n );
    m_pPIDtagHdg = ( PID* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDtagHdg, n + " not found" );

    n = "";
    pK->v ( "_Universe", &n );
    m_pU = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pU, n + ": not found" );

    return true;
}

bool _AProver_BRfollow::start ( void )
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AProver_BRfollow::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pD, -1 );
    NULL__ ( m_pPIDtagX, -1 );
    NULL__ ( m_pPIDtagHdg, -1 );
    NULL__ ( m_pU, -1 );

    return this->_StateBase::check();
}

void _AProver_BRfollow::update ( void )
{
    while(m_pT->bRun())
    {
        m_pT->autoFPSfrom();
        this->_StateBase::update();

        updateFollow();

        m_pT->autoFPSto();
    }
}

void _AProver_BRfollow::updateFollow ( void )
{
    IF_ ( check() <0 );
	IF_(!bActive());
    
    float dir = m_pD->getDirection();   //+/-1.0
    float nSpd = m_nSpd;
    
    _Object* pO = findTarget();
    if ( pO )
    {
        int iTag = pO->getTopClass();
        if(iTag == m_iTagStop)
        {
            nSpd = 0.0;
            m_errX = 0.0;
            m_errHdg = 0.0;
        }
        else if(iTag != m_iTag)
        {
            m_pD->setSteering(0.0);
            m_pD->setSpeed(0.0);
            m_iTag = iTag;
            m_pT->sleepT (m_tStop * USEC_1SEC);
            return;
        }
        else
        {
            m_errX = dir * (pO->getX() - m_tagTargetX);
            m_errHdg = dHdg<float>(0.0, pO->getRoll());
        }
        
    }

    m_nStr = m_pPIDtagX->update ( m_errX, 0.0, m_pT->getTstamp())
             + m_pPIDtagHdg->update ( m_errHdg, 0.0, m_pT->getTstamp());

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
    while ( ( pO = m_pU->get ( i++ ) ) != NULL )
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
    this->_StateBase::draw();
    drawActive();

   	addMsg("nSpd=" + f2str(m_nSpd) + ", nStr=" + f2str(m_nStr));
   	addMsg("errX=" + f2str(m_errX) + ", errHdg=" + f2str(m_errHdg));
}

}
