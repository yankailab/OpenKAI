#include "_APmav_relay.h"

namespace kai
{

	_APmav_relay::_APmav_relay()
	{
	}

	_APmav_relay::~_APmav_relay()
	{
	}

	bool _APmav_relay::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		const json &jc = jK(j, "channels");
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

	bool _APmav_relay::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_APmav_base", n);
		m_pAP = (_APmav_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmav_relay::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_relay::check(void)
	{
		NULL_F(m_pAP);
		NULL_F(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmav_relay::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateRelay();
		}
	}

	void _APmav_relay::updateRelay(void)
	{
		IF_(!check());

		_Mavlink *pMav = m_pAP->getMavlink();

		for (AP_relay s : m_vRelay)
		{
			pMav->clDoSetRelay(s.m_iChan, s.m_bRelay);
		}
	}

	void _APmav_relay::console(void *pConsole)
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
