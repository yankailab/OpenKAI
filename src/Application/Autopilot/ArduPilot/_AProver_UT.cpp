#include "_AProver_UT.h"

namespace kai
{

_AProver_UT::_AProver_UT()
{
	m_pAP = NULL;

    m_rcMode.init();
    m_rcDir.init();

	m_iPinLED = 21;
	m_iPinShutter = 10;
}

_AProver_UT::~_AProver_UT()
{
}

bool _AProver_UT::init(void* pKiss)
{
	IF_F(!this->_MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

    pK->v ( "iRCmodeChan", &m_rcMode.m_iChan );
    pK->a ( "vRCmodeDiv", &m_rcMode.m_vDiv );
    pK->v ( "iRCdirChan", &m_rcDir.m_iChan );
    pK->a ( "vRCdirDiv", &m_rcDir.m_vDiv );

    m_rcMode.setup();
    m_rcDir.setup();

	NULL_F(m_pMC);
    m_iMode.STANDBY = m_pMC->getMissionIdx ( "STANDBY" );
    m_iMode.FORWARD = m_pMC->getMissionIdx ( "FORWARD" );
    m_iMode.BACKWARD = m_pMC->getMissionIdx ( "BACKWARD" );
    IF_F ( !m_iMode.bValid() );

	pK->v<uint8_t>("iPinLED", &m_iPinLED);
	pK->v<uint8_t>("iPinShutter", &m_iPinShutter);

	string n;
	n = "";
	pK->v("_AP_base", &n );
	m_pAP = (_AP_base*) (pK->getInst( n ));
	IF_Fl(!m_pAP, n + ": not found");

	return true;
}

bool _AProver_UT::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: " << retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AProver_UT::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMav, -1);
	NULL__(m_pMC, -1);
	NULL__(m_pUdiv, -1);
	NULL__(m_pUtag, -1);

	return this->_MissionBase::check();
}

void _AProver_UT::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_MissionBase::update();

		updateMode();

		this->autoFPSto();
	}
}

void _AProver_UT::updateMode(void)
{
	IF_(check() < 0);
    
    uint32_t apMode = m_pAP->getApMode();
    
    uint16_t pwmMode = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
    IF_( pwmMode == UINT16_MAX );

    m_rcMode.pwm ( pwmMode );
    int iMode = m_rcMode.i();
    
    uint16_t pwmDir = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcDir.m_iChan );
    float dir = 0.0;
    if ( pwmDir != UINT16_MAX )
    {
        m_rcDir.pwm ( pwmDir );
        dir = ( float ) m_rcDir.i();
        dir -= 1.0;
    }

    switch ( iMode )
    {
    case 0:
        m_pMC->transit ( m_iMode.FORWARD );
        break;
    case 1:
        m_pMC->transit ( m_iMode.STANDBY );
        break;
    case 2:
        m_pMC->transit ( m_iMode.BACKWARD );
        break;
    default:
        m_pMC->transit ( m_iMode.STANDBY );
        break;
    }

    if ( apMode != AP_ROVER_MANUAL )
    {
        m_pMC->transit ( m_iMode.STANDBY );
    }


    //tag
	if(m_pUdiv->size() > 0)
	{
		if (!m_bDivider)
		{
			m_pMC->transit(m_iMode.STANDBY);
		}
		m_bDivider = true;
	}
}

void _AProver_UT::draw(void)
{
	this->_MissionBase::draw();
	IF_(check()<0);

}

}
