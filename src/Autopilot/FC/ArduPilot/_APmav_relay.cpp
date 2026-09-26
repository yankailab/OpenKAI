#include "_APmav_relay.h"

namespace kai
{

	_APmav_relay::_APmav_relay()
	{
	}

	_APmav_relay::~_APmav_relay()
	{
	}

	bool _APmav_relay::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		m_vRelay.clear();
		const json *pJc = jK(j, "channels");
		IF__(!pJc || !pJc->is_object(), true);
		const json &jc = *pJc;

		for (auto it = jc.begin(); it != jc.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			AP_relay s;
			s.m_configKey = it.key();
			s.init();
			jKv(Ji, "iChan", s.m_iChan);
			jKv(Ji, "bRelay", s.m_bRelay);
			m_vRelay.push_back(s);
		}

		return true;
	}

	bool _APmav_relay::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		json &channels = j["channels"];
		if (!channels.is_object())
		{
			channels = json::object();
		}
		for (size_t i = 0; i < m_vRelay.size(); ++i)
		{
			AP_relay &entry = m_vRelay[i];
			if (entry.m_configKey.empty())
			{
				entry.m_configKey = std::to_string(i);
				while (channels.contains(entry.m_configKey))
				{
					entry.m_configKey += "_";
				}
			}
			json &value = channels[entry.m_configKey];
			if (!value.is_object())
			{
				value = json::object();
			}
			value["iChan"] = entry.m_iChan;
			value["bRelay"] = entry.m_bRelay;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _APmav_relay::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_APmav_base", n);
		m_pAP = (_APmav_base *)(m_pM->findModule(n));
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
