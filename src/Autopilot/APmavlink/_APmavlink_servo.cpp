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

	bool _APmavlink_servo::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		const json &jc = j.at("channels");
		IF__(!jc.is_object(), true);

		for (auto it = jc.begin(); it != jc.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			AP_SERVO s;
			s.init();
			s.m_iChan = Ji.value("iChan", 9);
			s.m_pwm = Ji.value("pwm", 1500);
			m_vServo.push_back(s);
		}

		return true;
	}

	bool _APmavlink_servo::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = j.value("_APmavlink_base", "");
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmavlink_servo::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_servo::check(void)
	{
		NULL_F(m_pAP);
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
