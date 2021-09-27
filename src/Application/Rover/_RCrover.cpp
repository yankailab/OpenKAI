#include "_RCrover.h"

namespace kai
{

	_RCrover::_RCrover()
	{
		m_pS = NULL;
		m_pD = NULL;

		m_iSpd = 2;
		m_iSteer = 0;

		m_nSpd = 0.0;
		m_nSteer = 0.0;
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
		pK->v("nSpd", &m_nSpd);
		pK->v("nSteer", &m_nSteer);

		string n;

		n = "";
		pK->v("_SBus", &n);
		m_pS = (_SBus *)(pK->getInst(n));
		IF_Fl(!m_pS, n + ": not found");

		n = "";
		pK->v("_Drive", &n);
		m_pD = (_Drive *)(pK->getInst(n));
		IF_Fl(!m_pD, n + ": not found");

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

		m_pD->setSpeed(m_nSpd);
		m_pD->setSteering(m_nSteer);
//		m_pD->setDirection();

		return true;
	}

	void _RCrover::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		msgActive(pConsole);
#endif
	}

}
