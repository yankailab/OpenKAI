#include "_RCrover.h"

namespace kai
{

	_RCrover::_RCrover()
	{
		m_pS = NULL;
		m_pD = NULL;
		m_pA = NULL;

		m_iSpd = 2;
		m_iSteer = 0;
		m_iElev = 8;

		m_nSpd = 0.5;
		m_nSteer = 0.5;
		m_nElev = 0.5;
	}

	_RCrover::~_RCrover()
	{
	}

	bool _RCrover::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iSpd", &m_iSpd);
		pK->v("iSteer", &m_iSteer);
		pK->v("iElev", &m_iElev);
		pK->v("nSpd", &m_nSpd);
		pK->v("nSteer", &m_nSteer);
		pK->v("nElev", &m_nElev);

		string n;

		n = "";
		pK->v("_SBus", &n);
		m_pS = (_SBus *)(pK->getInst(n));
		IF_Fl(!m_pS, n + ": not found");

		n = "";
		pK->v("_Drive", &n);
		m_pD = (_Drive *)(pK->getInst(n));
		IF_Fl(!m_pD, n + ": not found");

		n = "";
		pK->v("_ActuatorBase", &n);
		m_pA = (_ActuatorBase *)(pK->getInst(n));
		IF_Fl(!m_pA, n + ": not found");

		return true;
	}

	bool _RCrover::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _RCrover::check(void)
	{
		NULL__(m_pS, -1);
		NULL__(m_pD, -1);

		return this->_StateBase::check();
	}

	void _RCrover::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateDrive();

			m_pT->autoFPSto();
		}
	}

	bool _RCrover::updateDrive(void)
	{
		IF_F(check() < 0);
		if (!bActive())
		{
			return false;
		}

		m_nSpd = m_pS->v(m_iSpd);
		m_nSteer = m_pS->v(m_iSteer);

		m_pD->setSpeed(m_nSpd - 0.5);
		m_pD->setSteering(m_nSteer - 0.5);
//		m_pD->setDirection();

		if(m_pA)
		{
			m_nElev = m_pS->v(m_iElev);
			if(m_nElev < 0.4)
				m_pA->setPtarget(0, -10);
			else if(m_nElev > 0.6)
				m_pA->setPtarget(0, 10);
			else
				m_pA->setStop();
		}

		return true;
	}

	void _RCrover::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		msgActive(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("nSpd: "	+ f2str(m_nSpd));
		pC->addMsg("nSteer: " + f2str(m_nSteer));
		pC->addMsg("nElev: " + f2str(m_nElev));
#endif
	}

}
