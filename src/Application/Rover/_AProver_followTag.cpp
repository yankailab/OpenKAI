#include "_AProver_followTag.h"

namespace kai
{

_AProver_followTag::_AProver_followTag()
{
    m_pAP = NULL;
    m_pD = NULL;
    m_pU = NULL;
    m_pPIDtagX = NULL;
    m_pPIDtagHdg = NULL;

    m_errX = 0.0;
    m_errHdg = 0.0;
    
    m_nSpd = 0.0;
    m_nStr = 0.0;
    m_tagTargetX = 0.5;
    m_iTagStop = -1;
    
    m_nSpdTurn = 0.05;
    m_errHdgStop = 10.0;
}

_AProver_followTag::~_AProver_followTag()
{
}

bool _AProver_followTag::init ( void* pKiss )
{
    IF_F ( !this->_StateBase::init ( pKiss ) );
    NULL_F( m_pSC );

    Kiss* pK = ( Kiss* ) pKiss;
    pK->v ( "nSpd", &m_nSpd );
    pK->v ( "nStr", &m_nStr );
    pK->v ( "tagTargetX", &m_tagTargetX );
    pK->v ( "iTagStop", &m_iTagStop );

    pK->v ( "nSpdTurn", &m_nSpdTurn );
    pK->v ( "errHdgStop", &m_errHdgStop );

    string n;

    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pAP, n + ": not found" );

    n = "";
    pK->v ( "_Drive", &n );
    m_pD = ( _Drive* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pD, n + ": not found" );

    n = "";
    pK->v ( "_Universe", &n );
    m_pU = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pU, n + ": not found" );

    n = "";
    pK->v ( "PIDtagX", &n );
    m_pPIDtagX = ( PID* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDtagX, n + " not found" );

    n = "";
    pK->v ( "PIDtagHdg", &n );
    m_pPIDtagHdg = ( PID* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDtagHdg, n + " not found" );

    return true;
}

bool _AProver_followTag::start ( void )
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AProver_followTag::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pD, -1 );
    NULL__ ( m_pU, -1 );
    NULL__ ( m_pPIDtagX, -1 );
    NULL__ ( m_pPIDtagHdg, -1 );

    return this->_StateBase::check();
}

void _AProver_followTag::update ( void )
{
    while(m_pT->bRun())
    {
        m_pT->autoFPSfrom();
        this->_StateBase::update();

        updateFollow();

        m_pT->autoFPSto();
    }
}

void _AProver_followTag::updateFollow ( void )
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
        }
        else
        {
            m_errX = dir * (pO->getX() - m_tagTargetX);
        }
        
        m_errHdg = dHdg<float>(0.0, pO->getRoll());
    }
    
    if(abs(m_errHdg) > m_errHdgStop)
    {
        nSpd = m_nSpdTurn;
        m_errX = 0.0;
    }

    m_nStr = dir * m_pPIDtagX->update ( m_errX, 0.0, m_pT->getTfrom())
             + dir * m_pPIDtagHdg->update ( m_errHdg, 0.0, m_pT->getTfrom());
    m_pD->setSteering(m_nStr);
    m_pD->setSpeed(nSpd);

}

_Object* _AProver_followTag::findTarget ( void )
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

void _AProver_followTag::console ( void* pConsole )
{
    NULL_(pConsole);
    this->_StateBase::console(pConsole);
    msgActive(pConsole);

	_Console *pC = (_Console *)pConsole;
   	pC->addMsg("nSpd=" + f2str(m_nSpd) + ", nStr=" + f2str(m_nStr));
   	pC->addMsg("errX=" + f2str(m_errX) + ", errHdg=" + f2str(m_errHdg));
}

}
