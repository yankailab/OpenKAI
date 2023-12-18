#include "_AP_mission.h"

namespace kai
{

	_AP_mission::_AP_mission()
	{
		m_pAP = NULL;
	}

	_AP_mission::~_AP_mission()
	{
	}

	bool _AP_mission::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool _AP_mission::link(void)
	{
		IF_F(!this->_StateBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		return true;
	}

	bool _AP_mission::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_mission::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);

		return this->_StateBase::check();
	}

	void _AP_mission::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			// this->_StateBase::update();
			// updateMission();

			m_pT->autoFPSto();
		}
	}

	void _AP_mission::updateMission(void)
	{
		IF_(check() < 0);

		int apMode = m_pAP->getApMode();
		string mission = m_pSC->getStateName();

		// if (apMode != AP_COPTER_GUIDED)
		// {
		// 	m_pSC->transit("STANDBY");
		// 	return;
		// }

		// if (mission == "STANDBY")
		// {
		// 	m_pSC->transit("TAKEOFF");
		// 	return;
		// }

		//	if(mission == "FOLLOW")
		//	{
		//		if(m_pAPdescent->m_bTarget)
		//			m_pSC->transit("DESCENT");
		//		return;
		//	}
		//
		//	if(mission == "DESCENT")
		//	{
		//		if(!m_pAPdescent->m_bTarget)
		//			m_pSC->transit("FOLLOW");
		//		return;
		//	}

		// if (mission == "RTH")
		// {
		// 	IF_(apMode != AP_COPTER_GUIDED);

		// 	m_pAP->setApMode(AP_COPTER_RTL);
		// 	return;
		// }
	}

	void _AP_mission::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		
		pC->addMsg("Test", 1);

	}

}
