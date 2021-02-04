#include "_AProver_UT.h"

namespace kai
{

_AProver_UT::_AProver_UT()
{
	m_pAP = NULL;
    m_pD = NULL;

    m_rcMode.init();
}

_AProver_UT::~_AProver_UT()
{
}

bool _AProver_UT::init(void* pKiss)
{
	IF_F(!this->_StateBase::init(pKiss));
   	NULL_F(m_pSC);

	Kiss* pK = (Kiss*) pKiss;

    pK->v ( "iRCmodeChan", &m_rcMode.m_iChan );
    pK->a ( "vRCmodeDiv", &m_rcMode.m_vDiv );
    m_rcMode.setup();

    m_iMode.MANUAL = m_pSC->getStateIdxByName ( "MANUAL" );
    m_iMode.FORWARD = m_pSC->getStateIdxByName ( "FORWARD" );
    m_iMode.BACKWARD = m_pSC->getStateIdxByName ( "BACKWARD" );
    IF_F ( !m_iMode.bValid() );

	string n;
	n = "";
	pK->v("_AP_base", &n );
	m_pAP = (_AP_base*) (pK->getInst( n ));
	IF_Fl(!m_pAP, n + ": not found");
    
    n = "";
	pK->v("Drive", &n);
	m_pD = ( _Drive*) (pK->getInst(n));
	IF_Fl(!m_pD, n + ": not found");

    
	return true;
}

bool _AProver_UT::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AProver_UT::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMav, -1);
	NULL__(m_pSC, -1);

	return this->_StateBase::check();
}

void _AProver_UT::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();
		this->_StateBase::update();

		updateMode();

		m_pT->autoFPSto();
	}
}

void _AProver_UT::updateMode(void)
{
	IF_(check() < 0);
    
    uint32_t apMode = m_pAP->getApMode();
    if ( apMode != AP_ROVER_MANUAL )
    {
        m_pSC->transit ( m_iMode.MANUAL );
        return;
    }
    
    float dir = 0.0;

    uint16_t pwmMode = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
    IF_ ( pwmMode == UINT16_MAX );
    m_rcMode.pwm ( pwmMode );
    int iMode = m_rcMode.i();
    switch ( iMode )
    {
    case 0:
        m_pSC->transit ( m_iMode.BACKWARD );
        dir = -1.0;
        break;
    case 1:
        m_pSC->transit ( m_iMode.MANUAL );
        break;
    case 2:
        m_pSC->transit ( m_iMode.FORWARD );
        dir = 1.0;
        break;
    default:
        m_pSC->transit ( m_iMode.MANUAL );
        break;
    }
    
    m_pD->setDirection(dir);
}

void _AProver_UT::draw(void)
{
	this->_StateBase::draw();
    drawActive();

   	addMsg("rcMode pwm=" + i2str(m_rcMode.m_pwm) + ", i=" + i2str(m_rcMode.i()));
}

}
