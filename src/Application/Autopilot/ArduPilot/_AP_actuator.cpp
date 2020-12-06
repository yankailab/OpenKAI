#include "../ArduPilot/_AP_actuator.h"

namespace kai
{

_AP_actuator::_AP_actuator()
{
	m_pAP = NULL;
    m_pAB = NULL;
    
    m_rcMode.init();
    m_rcStickV.init();
    m_rcStickH.init();    
}

_AP_actuator::~_AP_actuator()
{
}

bool _AP_actuator::init(void* pKiss)
{
	IF_F(!this->_MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

    pK->v ( "iRCmodeChan", &m_rcMode.m_iChan );
    pK->a ( "vRCmodeDiv", &m_rcMode.m_vDiv );
    pK->v ( "iRCstickV", &m_rcStickV.m_iChan );
    pK->v ( "iRCstickH", &m_rcStickH.m_iChan );

	string n = "";

    pK->v("_AP_base", &n );
	m_pAP = (_AP_base*) (pK->getInst( n ));
	IF_Fl(!m_pAP, n + ": not found");
    
   	pK->v("_ActuatorBase", &n );
	m_pAB = (_ActuatorBase*) (pK->getInst( n ));
	IF_Fl(!m_pAB, n + ": not found");

	return true;
}

bool _AP_actuator::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_actuator::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMav, -1);
	NULL__(m_pAB, -1);

	return this->_MissionBase::check();
}

void _AP_actuator::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_MissionBase::update();
		updateActuator();

		this->autoFPSto();
	}
}

void _AP_actuator::updateActuator(void)
{
	IF_(check() < 0);

    uint16_t pwm;
    
    pwm = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
    IF_ ( pwm == UINT16_MAX );
    m_rcMode.pwm ( pwm );
    int iMode = m_rcMode.i();
    
    pwm = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcStickV.m_iChan );
    IF_ ( pwm == UINT16_MAX );
    m_rcStickV.pwm ( pwm );
    
    pwm = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcStickH.m_iChan );
    IF_ ( pwm == UINT16_MAX );
    m_rcStickH.pwm ( pwm );

    m_pAB->power(iMode!=0?true:false);
    IF_(iMode != 0);
    
//    m_pAB->setPtarget(0, m_rcStickV.v());
//    m_pAB->setPtarget(1, m_rcStickH.v());
    
    float s;
    
    s= m_rcStickV.v();
    m_pAB->setStarget(0, s);

    s= m_rcStickH.v();
    m_pAB->setStarget(1, s);    
}

void _AP_actuator::draw(void)
{
	IF_(check() < 0);
	this->_MissionBase::draw();
	drawActive();

	addMsg("iMode: "+i2str(m_rcMode.i()), 1);
	addMsg("stickV: "+f2str(m_rcStickV.i()), 1);
	addMsg("stickH: "+f2str(m_rcStickH.i()), 1);
}

}
