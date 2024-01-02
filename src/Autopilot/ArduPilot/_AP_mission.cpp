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

/*		
		pC->addMsg("-----------", 1);
		pC->addMsg("GPS (RTK enabled)", 1);
		pC->addMsg("Sattelite number = 23", 1);
		pC->addMsg("Difference avg. = 0.0045 m", 1);
		pC->addMsg("Difference min. = 0.002 m", 1);
		pC->addMsg("Difference max. = 0.0049 m", 1);
		pC->addMsg("-----------", 1);
		pC->addMsg("LiDAR SLAM", 1);
		pC->addMsg("Difference avg. = 0.07 m", 1);
		pC->addMsg("Difference min. = 0.015 m", 1);
		pC->addMsg("Difference max. = 0.098 m", 1);
		pC->addMsg("-----------", 1);
		pC->addMsg("Windy condition", 1);
		pC->addMsg("Estimated wind speed = 3.5m/s", 1);
		pC->addMsg("Difference avg. = 0.07 m", 1);
		pC->addMsg("Difference min. = 0.05 m", 1);
		pC->addMsg("Difference max. = 0.09 m", 1);
		pC->addMsg("Estimated wind speed = 15m/s", 1);
		pC->addMsg("Difference avg. = 0.12 m", 1);
		pC->addMsg("Difference min. = 0.06 m", 1);
		pC->addMsg("Difference max. = 0.19 m", 1);
		pC->addMsg("-----------", 1);
		pC->addMsg("Target navigation", 1);
		pC->addMsg("Difference avg. = 0.06 m", 1);
		pC->addMsg("Difference min. = 0.05 m", 1);
		pC->addMsg("Difference max. = 0.08 m", 1);
		pC->addMsg("", 1);
*/

		// pC->addMsg("-----------", 1);
		// pC->addMsg("Vision detection", 1);
		// pC->addMsg("Platform: ARM NEON(Jetson)", 1);
		// pC->addMsg("Success prob. = 0.998", 1);
		// pC->addMsg("Detection FPS = 64", 1);
		// pC->addMsg("Tracking FPS = 129", 1);
		// pC->addMsg("Resolution = 1280x720", 1);
		// pC->addMsg("Inf. time (Latency) = 8ms", 1);
		// pC->addMsg("-----------", 1);
		// pC->addMsg("Vision detection", 1);
		// pC->addMsg("Platform: x86 (Intel SSE)", 1);
		// pC->addMsg("Success prob. = 0.998", 1);
		// pC->addMsg("Detection FPS = 109", 1);
		// pC->addMsg("Tracking FPS = 228", 1);
		// pC->addMsg("Resolution = 1280x720", 1);
		// pC->addMsg("Inf. time (Latency) = 4ms", 1);

		// pC->addMsg("-----------", 1);
		// pC->addMsg("Robotic arm", 1);
		// pC->addMsg("Torque = 850 N cm (85 Kg cm)", 1);
		// pC->addMsg("Rotational speed = 225 deg/s", 1);
		// pC->addMsg("Position difference = 0.0005 mm", 1);
		// pC->addMsg("Payload = 20Kg", 1);

	}

}
