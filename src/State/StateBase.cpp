#include "StateBase.h"

namespace kai
{

	StateBase::StateBase()
	{
		m_type = state_base;
		m_next = "";
		m_tStamp = 0;
		m_tTimeout = 0;

		reset();
	}

	StateBase::~StateBase()
	{
	}

	bool StateBase::init(void *pKiss)
	{
		IF_F(!this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("next", &m_next);

		return true;
	}

	bool StateBase::link(void)
	{
		IF_F(!this->BASE::link());
		Kiss *pK = (Kiss *)m_pKiss;

		vector<string> vS;

		pK->a("vModuleActive", &vS);
		for (string s : vS)
		{
			_ModuleBase* pM = (_ModuleBase *)(pK->getInst(s));
			IF_CONT(!pM);
			m_vpModuleActive.push_back(pM);
		}

		pK->a("vModuleSleep", &vS);
		for (string s : vS)
		{
			_ModuleBase* pM = (_ModuleBase *)(pK->getInst(s));
			IF_CONT(!pM);
			m_vpModuleSleep.push_back(pM);
		}

		return true;
	}

	void StateBase::reset(void)
	{
		m_tStart = 0;
		m_bComplete = false;
	}

	void StateBase::update(void)
	{
		IF_(m_bComplete);

		m_tStamp = getApproxTbootUs();
		if (m_tStart <= 0)
			m_tStart = m_tStamp;

		IF_(m_tTimeout <= 0);
		IF_(m_tStamp < m_tStart + m_tTimeout);
	}

	void StateBase::updateModules(void)
	{
		for(_ModuleBase* pM : m_vpModuleActive)
		{
			pM->run();
		}

		for(_ModuleBase* pM : m_vpModuleSleep)
		{
			pM->sleep();
		}
	}


	bool StateBase::bComplete(void)
	{
		return m_bComplete;
	}

	string StateBase::getNext(void)
	{
		return m_next;
	}

	STATE_TYPE StateBase::getType(void)
	{
		return m_type;
	}

	void StateBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->BASE::console(pConsole);

		// _Console *pC = (_Console *)pConsole;
		// if (!bActive())
		// 	pC->addMsg("[Inactive]", 0);
		// else
		// 	pC->addMsg("[ACTIVE]", 0);
	}

}
