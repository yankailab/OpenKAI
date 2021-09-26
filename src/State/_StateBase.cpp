#include "_StateBase.h"

namespace kai
{

	_StateBase::_StateBase()
	{
		m_pSC = NULL;
		m_iLastState = -1;
		m_bStateChanged = false;
	}

	_StateBase::~_StateBase()
	{
	}

	bool _StateBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n = "";
		pK->v("_StateControl", &n);
		m_pSC = (_StateControl *)(pK->getInst(n));
		NULL_T(m_pSC);

		vector<string> vAS;
		pK->a("vActiveState", &vAS);
		for (int i = 0; i < vAS.size(); i++)
		{
			int iS = m_pSC->getStateIdxByName(vAS[i]);
			if (iS < 0)
				continue;

			m_vActiveState.push_back(iS);
		}

		m_iLastState = -1;

		return true;
	}

	int _StateBase::check(void)
	{
		return this->_ModuleBase::check();
	}

	void _StateBase::update(void)
	{
		NULL_(m_pSC);
		int iState = m_pSC->getStateIdx();
		if (m_iLastState != iState)
		{
			m_bStateChanged = true;
			m_iLastState = iState;
		}
		else
		{
			m_bStateChanged = false;
		}
	}

	bool _StateBase::bActive(void)
	{
		NULL_T(m_pSC);
		IF_T(m_vActiveState.empty());

		int iMission = m_pSC->getStateIdx();
		for (int i : m_vActiveState)
		{
			if (iMission == i)
				return true;
		}

		return false;
	}

	bool _StateBase::bStateChanged(void)
	{
		return m_bStateChanged;
	}

	void _StateBase::msgActive(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!bActive())
			pC->addMsg("[Inactive]", 0);
		else
			pC->addMsg("[ACTIVE]", 0);
#endif
	}

	void _StateBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		msgActive(pConsole);
	}

}
