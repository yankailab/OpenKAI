#include "_AP_swarm.h"

namespace kai
{

	_AP_swarm::_AP_swarm()
	{
		m_pAP = NULL;
		m_bAutoArm = false;
		m_altAirborne = 20.0;
		m_dLanded = 5;
	}

	_AP_swarm::~_AP_swarm()
	{
	}

	bool _AP_swarm::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bAutoArm", &m_bAutoArm);
		pK->v("altAirborne", &m_altAirborne);
		pK->v("dLanded", &m_dLanded);

		return true;
	}

	bool _AP_swarm::link(void)
	{
		IF_F(!this->_StateBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		IF_F(!m_pSC);
		IF_F(!m_state.assign(m_pSC));

		return true;
	}

	bool _AP_swarm::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_swarm::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);
		NULL__(m_pSC, -1);

		return this->_StateBase::check();
	}

	void _AP_swarm::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_StateBase::update();
			updateSwarm();

			m_pT->autoFPSto();
		}
	}

	void _AP_swarm::updateSwarm(void)
	{
		IF_(check() < 0);
		m_state.update(m_pSC);

		int apMode = m_pAP->getApMode();
		bool bApArmed = m_pAP->bApArmed();
		float alt = m_pAP->getGlobalPos().w; // relative altitude

		// For manual reset
		if (apMode == AP_COPTER_STABILIZE)
		{
			m_pSC->transit(m_state.STANDBY);
			m_state.update(m_pSC);
		}

		// Standby
		if (m_state.bSTANDBY())
		{
			IF_(apMode != AP_COPTER_GUIDED);

			//            m_pSC->transit(m_state.TAKEOFF_REQUEST);
			//            m_state.update(m_pSC);
		}

		// Takeoff
		if (m_state.bTAKEOFF())
		{
			IF_(apMode != AP_COPTER_GUIDED);

			if (!bApArmed)
			{
				if (m_bAutoArm)
				{
					m_pAP->setApArm(true);
				}

				return;
			}

			m_pAP->m_pMav->clNavTakeoff(m_altAirborne + 1.0);

			IF_(alt < m_altAirborne);

			//			m_pSC->transit(m_state.AIRBORNE);
			//			m_state.update(m_pSC);
		}

		if (m_state.bAIRBORNE())
		{
			if (apMode == AP_COPTER_GUIDED)
				m_pAP->setApMode(AP_COPTER_AUTO);

			IF_(apMode != AP_COPTER_AUTO);

			IF_(alt > m_altAirborne);

//			m_pSC->transit(m_state.RTL);
//			m_state.update(m_pSC);
		}

		// RTL
		if (m_state.bRTL())
		{
			if (apMode == AP_COPTER_GUIDED)
				m_pAP->setApMode(AP_COPTER_RTL);

			// check if touched down
			IF_(bApArmed);

			m_pSC->transit(m_state.STANDBY);
			m_state.update(m_pSC);
		}
	}

}
