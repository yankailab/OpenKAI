#include "_AProver_UTfollowTag.h"

namespace kai
{

_AProver_UTfollowTag::_AProver_UTfollowTag()
{
    m_pAP = NULL;
    m_pD = NULL;
    m_pUside = NULL;
    m_pPIDside = NULL;

    m_errSide = 0.0;
    m_targetSide = 0.4;
    m_vSide.init(0.2, 0.8);
    
    m_pUdivider = NULL;
    m_bDivider = false;
    
    m_pUtag = NULL;
    m_iTag = -1;
    m_vTagX.init(0.2, 0.8);
    
    m_nSpd = 0.0;
    m_nStr = 0.0;
}

_AProver_UTfollowTag::~_AProver_UTfollowTag()
{
}

bool _AProver_UTfollowTag::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );

    Kiss* pK = ( Kiss* ) pKiss;
    pK->v ( "targetSide", &m_targetSide );
    pK->v ( "nSpd", &m_nSpd );
    pK->v ( "nStr", &m_nStr );
    pK->v ( "vTagX", &m_vTagX );
    pK->v ( "vSide", &m_vSide );
    
    IF_F(!m_pMC);
	IF_F(!m_iMission.assign(m_pMC));

    string n;

    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pAP, n + ": not found" );

    n = "";
    pK->v ( "Drive", &n );
    m_pD = ( Drive* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pD, n + ": not found" );

    n = "";
    pK->v ( "_UniverseSide", &n );
    m_pUside = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pUside, n + ": not found" );

    n = "";
    pK->v ( "PIDside", &n );
    m_pPIDside = ( PIDctrl* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDside, n + " not found" );

    n = "";
    pK->v ( "_UniverseDivider", &n );
    m_pUdivider = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pUdivider, n + ": not found" );

    n = "";
    pK->v ( "_UniverseTag", &n );
    m_pUtag = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pUtag, n + ": not found" );

    return true;
}

bool _AProver_UTfollowTag::start ( void )
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

int _AProver_UTfollowTag::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pD, -1 );
    NULL__ ( m_pUside, -1 );
    NULL__ ( m_pPIDside, -1 );
    NULL__ ( m_pUdivider, -1 );
    NULL__ ( m_pUtag, -1 );

    return this->_MissionBase::check();
}

void _AProver_UTfollowTag::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_MissionBase::update();

        updateFollow();

        this->autoFPSto();
    }
}

void _AProver_UTfollowTag::updateFollow ( void )
{
    IF_ ( check() <0 );
	IF_(!bActive());
    
    float dir = m_pD->getDirection();   //+/-1.0
    float nSpd = m_nSpd;
    
    _Object* pO;

    
    // follow the left side border of the multi-sheet
    pO = m_pUside->get(0);
    if ( pO && m_vSide.bInside(pO->getY()))
    {
        m_errSide = dir * (pO->getY() - m_targetSide );
    }
    m_nStr = dir * m_pPIDside->update ( m_errSide, 0.0, m_tStamp );

    
    // make a stop at divider
    pO = m_pUdivider->get(0);
    if ( pO )
    {
        if(!m_bDivider)
        {
            m_bDivider = true;
            nSpd = 0.0;
            m_nStr = 0.0;
            m_pMC->transit(m_iMission.DIVIDER);
        }
    }
    else
    {
        m_bDivider = false;
    }
    
    
    // make a stop at tag
    pO = m_pUtag->get(0);
    if ( pO )
    {
        if(m_iTag < 0 && m_vTagX.bInside(pO->getX()))
        {
            m_iTag = pO->getTopClass();
            nSpd = 0.0;
            m_nStr = 0.0;
            m_pMC->transit(m_iMission.FOLLOW);        
        }
    }
    else
    {
        m_iTag = -1;
    }

    m_pD->setSteering(m_nStr);
    m_pD->setSpeed(nSpd);    
}

void _AProver_UTfollowTag::draw ( void )
{
    this->_MissionBase::draw();
    drawActive();

   	addMsg("nSpd=" + f2str(m_nSpd) + ", nStr=" + f2str(m_nStr));
   	addMsg("errSide=" + f2str( m_errSide ));
}

}
