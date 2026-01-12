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

	int _APmavlink_servo::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		string n;
		n = "";
		= j.value("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL__(m_pAP);

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

		return true;
	}

	int _APmavlink_servo::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_servo::check(void)
	{
		NULL__(m_pAP);
		NULL__(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmavlink_servo::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateServo();

		}
	}

	void _APmavlink_servo::updateServo(void)
	{
		IF_(!check());

		_Mavlink *pMav = m_pAP->getMavlink();

		for (AP_SERVO s : m_vServo)
		{
			pMav->clDoSetServo(s.m_iChan, s.m_pwm);
		}
	}

	void _APmavlink_servo::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(!check());

		for (AP_SERVO s : m_vServo)
		{
			((_Console *)pConsole)->addMsg("Chan:" + i2str((int)s.m_iChan) + ", pwm=" + i2str((int)s.m_pwm), 1);
		}
	}

}
