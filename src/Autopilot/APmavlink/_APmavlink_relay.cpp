#include "_APmavlink_relay.h"

namespace kai
{

	_APmavlink_relay::_APmavlink_relay()
	{
		m_pAP = nullptr;
	}

	_APmavlink_relay::~_APmavlink_relay()
	{
	}

	int _APmavlink_relay::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

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
		= j.value("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL__(m_pAP);

		return true;
	}

	int _APmavlink_relay::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_relay::check(void)
	{
		NULL__(m_pAP);
		NULL__(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmavlink_relay::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateRelay();

		}
	}

	void _APmavlink_relay::updateRelay(void)
	{
		IF_(!check());

		_Mavlink *pMav = m_pAP->getMavlink();

		for (AP_relay s : m_vRelay)
		{
			pMav->clDoSetRelay(s.m_iChan, s.m_bRelay);
		}
	}

	void _APmavlink_relay::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(!check());

		for (AP_relay s : m_vRelay)
		{
			((_Console *)pConsole)->addMsg("Chan:" + i2str((int)s.m_iChan) + ", bRelay=" + i2str((int)s.m_bRelay), 1);
		}
	}

}
