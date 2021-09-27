#include "_AProver_KU.h"

namespace kai
{

_AProver_KU::_AProver_KU()
{
	m_pAP = NULL;
    m_pD = NULL;
    
    m_rcMode.init();
    m_rcDir.init();
}

_AProver_KU::~_AProver_KU()
{
}

bool _AProver_KU::init(void* pKiss)
{
	IF_F(!this->_StateBase::init(pKiss));
	NULL_F(m_pSC);

    Kiss* pK = (Kiss*)pKiss;
    
    pK->v ( "iRCmodeChan", &m_rcMode.m_iChan );
    pK->a ( "vRCmodeDiv", &m_rcMode.m_vDiv );
    pK->v ( "iRCdirChan", &m_rcDir.m_iChan );
    pK->a ( "vRCdirDiv", &m_rcDir.m_vDiv );

    m_rcMode.update();
    m_rcDir.update();

    m_iMode.STANDBY = m_pSC->getStateIdxByName ( "STANDBY" );
    m_iMode.HYBRID = m_pSC->getStateIdxByName ( "HYBRID" );
    m_iMode.AUTO = m_pSC->getStateIdxByName ( "AUTO" );
    IF_F ( !m_iMode.bValid() );

	string n;
	n = "";
	pK->v("_AP_base", &n);
	m_pAP = (_AP_base*) (pK->getInst(n));
	IF_Fl(!m_pAP, n + ": not found");

	n = "";
	pK->v("Drive", &n);
	m_pD = ( _Drive*) (pK->getInst(n));
	IF_Fl(!m_pD, n + ": not found");

	return true;
}

bool _AProver_KU::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AProver_KU::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMav, -1);
	NULL__(m_pSC, -1);
	NULL__(m_pD, -1);

	return this->_StateBase::check();
}

void _AProver_KU::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();
		this->_StateBase::update();

        updateMode();

		m_pT->autoFPSto();
	}
}

void _AProver_KU::updateMode ( void )
{
    IF_ ( check() <0 );

    uint16_t pwmDir = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcDir.m_iChan );
    float dir = 0.0;
    if ( pwmDir != UINT16_MAX )
    {
        m_rcDir.set ( pwmDir );
        dir = ( float ) m_rcDir.i();
        dir -= 1.0;
    }
    m_pD->setDirection(dir);

    uint32_t apMode = m_pAP->getApMode();
    if ( apMode != AP_ROVER_MANUAL )
    {
        m_pSC->transit ( m_iMode.STANDBY );
        return;
    }
    
    uint16_t pwmMode = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
    IF_ ( pwmMode == UINT16_MAX );
    m_rcMode.set ( pwmMode );
    int iMode = m_rcMode.i();
    switch ( iMode )
    {
    case 0:
        m_pSC->transit ( m_iMode.STANDBY );
        break;
    case 1:
        m_pSC->transit ( m_iMode.HYBRID );
        break;
    case 2:
        m_pSC->transit ( m_iMode.AUTO );
        break;
    default:
        m_pSC->transit ( m_iMode.STANDBY );
        break;
    }
}

void _AProver_KU::console(void* pConsole)
{
    NULL_(pConsole);
	this->_StateBase::console(pConsole);
    msgActive(pConsole);

	_Console *pC = (_Console *)pConsole;
   	pC->addMsg("rcMode pwm=" + i2str(m_rcMode.m_raw) + ", i=" + i2str(m_rcMode.i()));
   	pC->addMsg("rcDir pwm=" + i2str(m_rcDir.m_raw) + ", i=" + i2str(m_rcDir.i()));
}

}
