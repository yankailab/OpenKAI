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

	bool _APmavlink_relay::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		const json &jc = j.at("channels");
		IF__(!jc.is_object(), true);

		for (auto it = jc.begin(); it != jc.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			AP_relay s;
			s.init();
			jKv(Ji, "iChan", s.m_iChan);
			jKv(Ji, "bRelay", s.m_bRelay);
			m_vRelay.push_back(s);
		}

		return true;
	}

	bool _APmavlink_relay::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_APmavlink_base", n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmavlink_relay::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_relay::check(void)
	{
		NULL_F(m_pAP);
		NULL_F(m_pAP->getMavlink());

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
