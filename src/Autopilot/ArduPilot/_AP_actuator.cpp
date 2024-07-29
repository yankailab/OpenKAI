#include "../ArduPilot/_AP_actuator.h"

namespace kai
{

	_AP_actuator::_AP_actuator()
	{
		m_pAP = nullptr;
		m_pAB1 = nullptr;
		m_pAB2 = nullptr;

		m_rcMode.update();
		m_rcStickV.update();
		m_rcStickH.update();
	}

	_AP_actuator::~_AP_actuator()
	{
	}

	int _AP_actuator::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iRCmodeChan", &m_rcMode.m_iChan);
		pK->a("vRCmodeDiv", &m_rcMode.m_vDiv);
		pK->v("iRCstickV", &m_rcStickV.m_iChan);
		pK->v("iRCstickH", &m_rcStickH.m_iChan);

		pK->a("vRCmodeDiv", &m_rcStickV.m_vDiv);
		pK->a("vRCmodeDiv", &m_rcStickH.m_vDiv);

		m_rcMode.update();
		m_rcStickV.update();
		m_rcStickH.update();

		string n = "";

		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		pK->v("_ActuatorBase1", &n);
		m_pAB1 = (_ActuatorBase *)(pK->findModule(n));
		NULL__(m_pAB1, OK_ERR_NOT_FOUND);

		pK->v("_ActuatorBase2", &n);
		m_pAB2 = (_ActuatorBase *)(pK->findModule(n));
		NULL__(m_pAB2, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _AP_actuator::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _AP_actuator::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);
		NULL__(m_pAB1, OK_ERR_NULLPTR);
		NULL__(m_pAB2, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _AP_actuator::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateActuator();

			m_pT->autoFPSto();
		}
	}

	void _AP_actuator::updateActuator(void)
	{
		IF_(check() != OK_OK);

		uint16_t pwm;

		// pwm = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
		// IF_ ( pwm == UINT16_MAX );
		// m_rcMode.pwm ( pwm );
		// int iMode = m_rcMode.i();

		pwm = m_pAP->m_pMav->m_rcChannels.getRC(m_rcStickV.m_iChan);
		IF_(pwm == UINT16_MAX);
		m_rcStickV.set(pwm);

		pwm = m_pAP->m_pMav->m_rcChannels.getRC(m_rcStickH.m_iChan);
		IF_(pwm == UINT16_MAX);
		m_rcStickH.set(pwm);

		// m_pAB1->power(iMode!=0?true:false);
		// IF_(iMode == 0);

		int i;

		i = m_rcStickV.i();
		if (i == 1)
		{
			m_pAB1->setStop();
		}
		else
		{
			m_pAB1->setPtarget(0, (i - 1) * 10);
		}

		i = m_rcStickH.i();
		if (i == 1)
		{
			m_pAB2->setStop();
		}
		else
		{
			m_pAB2->setPtarget(0, (i - 1) * 10);
		}
	}

	void _AP_actuator::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() != OK_OK);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iMode: " + i2str(m_rcMode.i()), 1);
		pC->addMsg("stickV = " + f2str(m_rcStickV.v()), 1);
		pC->addMsg("stickH = " + f2str(m_rcStickH.v()), 1);
	}

}
