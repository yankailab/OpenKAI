#include "_APmav_servo.h"

namespace kai
{

	_APmav_servo::_APmav_servo()
	{
	}

	_APmav_servo::~_APmav_servo()
	{
	}

	bool _APmav_servo::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		const json *pJc = jK(j, "channels");
		IF__(!pJc || !pJc->is_object(), true);
		const json &jc = *pJc;

		for (auto it = jc.begin(); it != jc.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			AP_SERVO s;
			s.init();
			jKv(Ji, "iChan", s.m_iChan);
			jKv(Ji, "pwm", s.m_pwm);
			m_vServo.push_back(s);
		}

		return true;
	}

	bool _APmav_servo::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_APmav_base", n);
		m_pAP = (_APmav_base *)(m_pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmav_servo::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_servo::check(void)
	{
		NULL_F(m_pAP);
		IF_F(!m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmav_servo::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateServo();
		}
	}

	void _APmav_servo::updateServo(void)
	{
		IF_(!check());

		_Mavlink *pMav = m_pAP->getMavlink();

		for (AP_SERVO s : m_vServo)
		{
			pMav->clDoSetServo(s.m_iChan, s.m_pwm);
		}
	}

	void _APmav_servo::console(void *pConsole)
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
