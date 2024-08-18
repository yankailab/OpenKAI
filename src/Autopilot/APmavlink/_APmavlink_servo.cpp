#include "_APmavlink_servo.h"

namespace kai
{

	_APmavlink_servo::_APmavlink_servo()
	{
		m_pAP = nullptr;
	}

	_APmavlink_servo::~_APmavlink_servo()
	{
	}

	int _APmavlink_servo::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
    	
		string n;
		n = "";
		pK->v("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		Kiss *pKc = pK->child("channels");
		NULL__(pKc, OK_OK);

		int i = 0;
		while (1)
		{
			Kiss *pS = pKc->child(i++);
			if (pS->empty())
				break;

			AP_SERVO s;
			s.init();
			pS->v("iChan", &s.m_iChan);
			pS->v("pwm", &s.m_pwm);
			m_vServo.push_back(s);
		}

		return OK_OK;
	}

	int _APmavlink_servo::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_servo::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _APmavlink_servo::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateServo();

			m_pT->autoFPSto();
		}
	}

	void _APmavlink_servo::updateServo(void)
	{
		IF_(check() != OK_OK);

		_Mavlink *pMav = m_pAP->m_pMav;

		for (AP_SERVO s : m_vServo)
		{
			pMav->clDoSetServo(s.m_iChan, s.m_pwm);
		}
	}

	void _APmavlink_servo::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() != OK_OK);

		for (AP_SERVO s : m_vServo)
		{
			((_Console *)pConsole)->addMsg("Chan:" + i2str((int)s.m_iChan) + ", pwm=" + i2str((int)s.m_pwm), 1);
		}
	}

}
