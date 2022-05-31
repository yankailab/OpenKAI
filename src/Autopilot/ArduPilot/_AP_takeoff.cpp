#include "../ArduPilot/_AP_takeoff.h"

namespace kai
{

	_AP_takeoff::_AP_takeoff()
	{
		m_pAP = NULL;
		m_apMode = AP_COPTER_GUIDED;
		m_dAlt = INT16_MAX;
	}

	_AP_takeoff::~_AP_takeoff()
	{
	}

	bool _AP_takeoff::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("apMode", &m_apMode);

		string n;
		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		return true;
	}

	bool _AP_takeoff::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_takeoff::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);

		return this->_StateBase::check();
	}

	void _AP_takeoff::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_StateBase::update();
			updateMission();

			m_pT->autoFPSto();
		}
	}

	void _AP_takeoff::updateMission(void)
	{
		IF_(check() < 0);
		IF_(!bActive());
		if (m_apMode >= 0)
		{
			IF_(m_pAP->getApMode() != m_apMode);
		}

		State *pM = m_pSC->getState();
		NULL_(pM);
		IF_(pM->type() != state_takeoff);

		Takeoff *pTO = (Takeoff *)pM;
		m_dAlt = (float)m_pAP->m_vGlobalPos.w - pTO->m_alt;
		if (m_dAlt > -0.5)
		{
			pTO->complete();
			return;
		}

		m_pAP->m_pMav->clNavTakeoff(pTO->m_alt);
	}

	void _AP_takeoff::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() < 0);
		this->_StateBase::console(pConsole);

		((_Console *)pConsole)->addMsg("alt = " + f2str(m_pAP->m_vGlobalPos.w) + ", dAlt = " + f2str(m_dAlt));
	}

}
