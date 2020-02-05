#include "../ArduPilot/_AP_servo.h"

namespace kai
{

_AP_servo::_AP_servo()
{
	m_pAP = NULL;
}

_AP_servo::~_AP_servo()
{
}

bool _AP_servo::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	Kiss** pItr = pK->getChildItr();
	int i=0;
	while (pItr[i])
	{
		Kiss* pS = pItr[i++];
		AP_SERVO s;
		s.init();
		pS->v("iChan",&s.m_iChan);
		pS->v("pwm",&s.m_pwm);
		m_vServo.push_back(s);
	}

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_AP_base", &iName));
	m_pAP = (_AP_base*) (pK->root()->getChildInst(iName));

	return true;
}

bool _AP_servo::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_servo::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_AutopilotBase::check();
}

void _AP_servo::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateServo();

		this->autoFPSto();
	}
}

void _AP_servo::updateServo(void)
{
	IF_(check()<0);
	IF_(!bActive());

	_Mavlink* pMav = m_pAP->m_pMavlink;

	for(AP_SERVO s : m_vServo)
	{
		pMav->clDoSetServo(s.m_iChan, s.m_pwm);
	}
}

void _AP_servo::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);

	for(AP_SERVO s : m_vServo)
	{
		addMsg("Chan:" + i2str((int)s.m_iChan) + ", pwm=" + i2str((int)s.m_pwm), 1);
	}

}

}
