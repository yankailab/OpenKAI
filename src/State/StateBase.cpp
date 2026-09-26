#include "StateBase.h"

namespace kai
{

	StateBase::StateBase()
	{
		m_class = "StateBase";
		reset();
	}

	StateBase::~StateBase()
	{
	}

	bool StateBase::loadConfig(void)
	{
		IF_F(!this->BASE::loadConfig());
		json &j = *m_pJ;

		jKv(j, "next", m_next);

		return true;
	}

	bool StateBase::link(void)
	{
		IF_F(!this->BASE::link());
		const json &j = *m_pJ;

		m_vpModuleResume.clear();
		m_vpModulePause.clear();
		vector<string> vS;

		vS.clear();
		jKv(j, "vModuleResume", vS);
		for (string n : vS)
		{
			_ModuleBase *pB = (_ModuleBase *)(m_pM->findModule(n));
			IF_CONT(!pB);
			m_vpModuleResume.push_back(pB);
		}

		vS.clear();
		jKv(j, "vModulePause", vS);
		for (string n : vS)
		{
			_ModuleBase *pB = (_ModuleBase *)(m_pM->findModule(n));
			IF_CONT(!pB);
			m_vpModulePause.push_back(pB);
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

		m_tStamp = getTns();
		if (m_tStart <= 0)
			m_tStart = m_tStamp;

		IF_(m_tTimeout <= 0);
		IF_(m_tStamp < m_tStart + m_tTimeout);
	}

	void StateBase::updateModules(void)
	{
		for (_ModuleBase *pM : m_vpModulePause)
		{
			pM->pause();
		}

		for (_ModuleBase *pM : m_vpModuleResume)
		{
			pM->resume();
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
