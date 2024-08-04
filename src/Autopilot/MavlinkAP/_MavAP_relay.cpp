#include "_MavAP_relay.h"

namespace kai
{

	_MavAP_relay::_MavAP_relay()
	{
		m_pAP = nullptr;
	}

	_MavAP_relay::~_MavAP_relay()
	{
	}

	int _MavAP_relay::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
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
		pK->v("_MavAP_base", &n);
		m_pAP = (_MavAP_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _MavAP_relay::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _MavAP_relay::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _MavAP_relay::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateRelay();

			m_pT->autoFPSto();
		}
	}

	void _MavAP_relay::updateRelay(void)
	{
		IF_(check() != OK_OK);

		_Mavlink *pMav = m_pAP->m_pMav;

		for (AP_relay s : m_vRelay)
		{
			pMav->clDoSetRelay(s.m_iChan, s.m_bRelay);
		}
	}

	void _MavAP_relay::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() != OK_OK);

		for (AP_relay s : m_vRelay)
		{
			((_Console *)pConsole)->addMsg("Chan:" + i2str((int)s.m_iChan) + ", bRelay=" + i2str((int)s.m_bRelay), 1);
		}
	}

}
