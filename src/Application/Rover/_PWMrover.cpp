#include "_PWMrover.h"

namespace kai
{

	_PWMrover::_PWMrover()
	{
		m_pDV = NULL;
		m_pU = NULL;
		m_pP = NULL;
		m_pD = NULL;

		m_iSpd = 2;
		m_iSteer = 0;

		m_nSpd = 0.5;
		m_nSteer = 0.5;
	}

	_PWMrover::~_PWMrover()
	{
	}

	bool _PWMrover::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iSpd", &m_iSpd);
		pK->v("iSteer", &m_iSteer);
		pK->v("nSpd", &m_nSpd);
		pK->v("nSteer", &m_nSteer);

		string n;

		n = "";
		pK->v("_DepthVisionBase", &n);
		m_pDV = (_DepthVisionBase *)(pK->getInst(n));
		IF_Fl(!m_pDV, n + ": not found");

		n = "";
		pK->v("_Universe", &n);
		m_pU = (_Universe *)(pK->getInst(n));
		IF_Fl(!m_pU, n + ": not found");

		n = "";
		pK->v("_ProtocolBase", &n);
		m_pP = (_ProtocolBase *)(pK->getInst(n));
		IF_Fl(!m_pP, n + ": not found");

		n = "";
		pK->v("_Drive", &n);
		m_pD = (_Drive *)(pK->getInst(n));
		IF_Fl(!m_pD, n + ": not found");

		return true;
	}

	bool _PWMrover::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _PWMrover::check(void)
	{
		NULL__(m_pDV, -1);
		NULL__(m_pU, -1);
		NULL__(m_pP, -1);
		NULL__(m_pD, -1);

		return this->_StateBase::check();
	}

	void _PWMrover::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateDrive();

			m_pT->autoFPSto();
		}
	}

	bool _PWMrover::updateDrive(void)
	{
		IF_F(check() < 0);
		if (!bActive())
		{
			return false;
		}

		// m_nSpd = m_pS->v(m_iSpd);
		// m_nSteer = m_pS->v(m_iSteer);

		// m_pD->setSpeed(m_nSpd - 0.5);
		// m_pD->setSteering(m_nSteer - 0.5);
//		m_pD->setDirection();


		return true;
	}

	void _PWMrover::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		msgActive(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("nSpd: "	+ f2str(m_nSpd));
		pC->addMsg("nSteer: " + f2str(m_nSteer));
#endif
	}

}
