#include "_AP_relay.h"

namespace kai
{

	_AP_relay::_AP_relay()
	{
		m_pAP = NULL;
	}

	_AP_relay::~_AP_relay()
	{
	}

	bool _AP_relay::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
    	

		int i = 0;
		while (1)
		{
			Kiss *pR = pK->child(i++);
			if (pR->empty())
				break;

			AP_relay r;
			r.init();
			pR->v("iChan", &r.m_iChan);
			pR->v("bRelay", &r.m_bRelay);
			m_vRelay.push_back(r);
		}

		string n;
		n = "";
		F_ERROR_F(pK->v("_AP_base", &n));
		m_pAP = (_AP_base *)(pK->findModule(n));

		return true;
	}

	bool _AP_relay::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_relay::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);

		return this->_ModuleBase::check();
	}

	void _AP_relay::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateRelay();

			m_pT->autoFPSto();
		}
	}

	void _AP_relay::updateRelay(void)
	{
		IF_(check() < 0);

		_Mavlink *pMav = m_pAP->m_pMav;

		for (AP_relay s : m_vRelay)
		{
			pMav->clDoSetRelay(s.m_iChan, s.m_bRelay);
		}
	}

	void _AP_relay::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() < 0);

		for (AP_relay s : m_vRelay)
		{
			((_Console *)pConsole)->addMsg("Chan:" + i2str((int)s.m_iChan) + ", bRelay=" + i2str((int)s.m_bRelay), 1);
		}
	}

}
